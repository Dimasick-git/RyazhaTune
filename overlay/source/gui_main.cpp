#include "gui_main.hpp"

#include "elm_overlayframe.hpp"
#include "elm_volume.hpp"
#include "gui_browser.hpp"
#include "gui_playlist.hpp"
#include "pm/pm.hpp"
#include "config/config.hpp"
#include <switch.h>

namespace {
    constexpr const size_t num_steps = 20;
}

MainGui::MainGui() {
    m_status_bar    = new StatusBar();
}

tsl::elm::Element *MainGui::createUI() {
    auto frame = new SysTuneOverlayFrame();
    auto list  = new tsl::elm::List();

    u64 pid{}, tid{};
    pm::getCurrentPidTid(&pid, &tid);

    /* Current track. */
    list->addItem(this->m_status_bar, tsl::style::ListItemDefaultHeight * 2);

    /* Playlist. */
    auto queue_button = new tsl::elm::ListItem("Плейлист");
    queue_button->setClickListener([](u64 keys) {
        if (keys & HidNpadButton_A) {
            tsl::changeTo<PlaylistGui>();
            return true;
        }
        return false;
    });
    list->addItem(queue_button);

    /* Browser. */
    auto browser_button = new tsl::elm::ListItem("Браузер музыки");
    browser_button->setClickListener([](u64 keys) {
        if (keys & HidNpadButton_A) {
            tsl::changeTo<BrowserGui>();
            return true;
        }
        return false;
    });
    list->addItem(browser_button);

    list->addItem(new tsl::elm::CategoryHeader("Настройки"));

    /* Whitelist mode toggle */
    auto whitelist_mode = new tsl::elm::ToggleListItem("Белый список", config::get_whitelist_mode());
    whitelist_mode->setStateChangedListener([](bool state) {
        config::set_whitelist_mode(state);
    });
    list->addItem(whitelist_mode);

    /* (Удалено) Режим проигрывания во время загрузки игры */

    /* Current game list management (whitelist/blacklist) */
    if (tid != 0) {
        auto current_whitelisted = config::get_title_whitelist(tid);
        auto current_blacklisted = config::get_title_blacklist(tid);

        auto wl_toggle = new tsl::elm::ToggleListItem("Текущая игра: Белый список", current_whitelisted);
        auto bl_toggle = new tsl::elm::ToggleListItem("Текущая игра: Чёрный список", current_blacklisted);

        wl_toggle->setStateChangedListener([tid, bl_toggle, frame](bool state) {
            config::set_title_whitelist(tid, state);
            if (state) {
                config::set_title_blacklist(tid, false);
                bl_toggle->setState(false);
                frame->setToast("Белый список", "Игра добавлена в белый список");
            } else {
                frame->setToast("Белый список", "Игра удалена из белого списка");
            }
        });

        bl_toggle->setStateChangedListener([tid, wl_toggle, frame](bool state) {
            config::set_title_blacklist(tid, state);
            if (state) {
                config::set_title_whitelist(tid, false);
                wl_toggle->setState(false);
                frame->setToast("Чёрный список", "Игра добавлена в чёрный список");
            } else {
                frame->setToast("Чёрный список", "Игра удалена из чёрного списка");
            }
        });

        list->addItem(wl_toggle);
        list->addItem(bl_toggle);
    }

    /* Volume indicator */
    list->addItem(new tsl::elm::CategoryHeader("Громкость"));

    /* Get initial volume. */
    float tune_volume = 1.f;
    float title_volume = 1.f;
    float default_title_volume = 1.f;

    tuneGetVolume(&tune_volume);
    tuneGetTitleVolume(&title_volume);
    tuneGetDefaultTitleVolume(&default_title_volume);

    auto tune_volume_slider = new ElmVolume("\uE13C", "Громкость музыки", num_steps);
    tune_volume_slider->setProgress(tune_volume * num_steps);
    tune_volume_slider->setValueChangedListener([](u8 value){
        const float volume = float(value) / float(num_steps);
        tuneSetVolume(volume);
    });
    list->addItem(tune_volume_slider);

    // empty pid means we are qlaunch :)
    if (tid && pid) {
        auto title_volume_slider = new ElmVolume("\uE13C", "Громкость игры", num_steps);
        title_volume_slider->setProgress(title_volume * num_steps);
        title_volume_slider->setValueChangedListener([tid](u8 value){
            const float volume = float(value) / float(num_steps);
            tuneSetTitleVolume(volume);
            config::set_title_volume(tid, volume);
        });
        list->addItem(title_volume_slider);
    }

    auto default_title_volume_slider = new ElmVolume("\uE13C", "Громкость игры (по умолчанию)", num_steps);
    default_title_volume_slider->setProgress(default_title_volume * num_steps);
    default_title_volume_slider->setValueChangedListener([](u8 value){
        const float volume = float(value) / float(num_steps);
        tuneSetDefaultTitleVolume(volume);
    });
    list->addItem(default_title_volume_slider);

    list->addItem(new tsl::elm::CategoryHeader("Воспроизведение / Пауза"));

    /* Autoplay on boot toggle */
    auto tune_autoplay = new tsl::elm::ToggleListItem("Автовоспроизведение при запуске", config::get_autoplay_enabled(), "Вкл", "Выкл");
    tune_autoplay->setStateChangedListener([](bool new_value) {
        config::set_autoplay_enabled(new_value);
        if (new_value) {
            tunePlay();
        }
    });
    list->addItem(tune_autoplay);

    /* Unified music toggle: per-title when игра активна, иначе — настройка по умолчанию */
    bool initial_music_enabled = (tid != 0) ? config::get_title_enabled(tid)
                                            : config::get_title_enabled_default();
    auto tune_play = new tsl::elm::ToggleListItem("Музыка", initial_music_enabled, "Воспроизвести", "Пауза");
    tune_play->setStateChangedListener([tid](bool new_value) {
        if (tid != 0) {
            config::set_title_enabled(tid, new_value);
        } else {
            config::set_title_enabled_default(new_value);
        }
        if (new_value) {
            tunePlay();
        } else {
            tunePause();
        }
    });
    list->addItem(tune_play);

    list->addItem(new tsl::elm::CategoryHeader("Прочее"));

    /* Scan and add all music recursively */
    auto scan_all_button = new tsl::elm::ListItem("Сканировать всю музыку");
    scan_all_button->setClickListener([frame](u64 keys){
        if (keys & HidNpadButton_A) {
            FsFileSystem fs;
            Result rc = fsOpenSdCardFileSystem(&fs);
            if (R_FAILED(rc)) {
                frame->setToast("Ошибка", "Не удалось открыть SD-карту");
                return true;
            }

            s64 added = 0;

            auto Supports = [](const char* name){
                auto endsWith = [](const char* n, const char* ext){
                    size_t ln = std::strlen(n), le = std::strlen(ext);
                    if (ln < le) return false;
                    return strcasecmp(n + ln - le, ext) == 0;
                };
                bool ok = false;
#ifdef WANT_MP3
                ok = ok || endsWith(name, ".mp3");
#endif
#ifdef WANT_FLAC
                ok = ok || endsWith(name, ".flac");
#endif
#ifdef WANT_WAV
                ok = ok || endsWith(name, ".wav") || endsWith(name, ".wave");
#endif
                return ok;
            };

            std::function<void(const char*)> walk;
            walk = [&](const char* dirPath){
                FsDir dir;
                if (R_FAILED(fsFsOpenDirectory(&fs, dirPath, FsDirOpenMode_ReadDirs | FsDirOpenMode_ReadFiles, &dir))) return;
                tsl::hlp::ScopeGuard guard([&]{ fsDirClose(&dir); });

                s64 count = 0;
                FsDirectoryEntry entry;
                while (R_SUCCEEDED(fsDirRead(&dir, &count, 1, &entry)) && count) {
                    if (entry.type == FsDirEntryType_Dir) {
                        char child[FS_MAX_PATH];
                        std::snprintf(child, sizeof(child), "%s%s/", dirPath, entry.name);
                        walk(child);
                    } else if (Supports(entry.name)) {
                        char full[FS_MAX_PATH];
                        std::snprintf(full, sizeof(full), "%s%s", dirPath, entry.name);
                        if (R_SUCCEEDED(tuneEnqueue(full, TuneEnqueueType_Back))) added++;
                    }
                }
            };

            // Prefer "/music/" if present, else root
            FsDir testDir;
            if (R_SUCCEEDED(fsFsOpenDirectory(&fs, "/music/", FsDirOpenMode_ReadDirs, &testDir))) {
                fsDirClose(&testDir);
                walk("/music/");
            } else {
                walk("/");
            }

            fsFsClose(&fs);

            char buf[128];
            std::snprintf(buf, sizeof(buf), "Найдено и добавлено %lld треков", (long long)added);
            frame->setToast("Плейлист обновлён", buf);
            return true;
        }
        return false;
    });
    list->addItem(scan_all_button);

    auto exit_button = new tsl::elm::ListItem("Закрыть RyazhTune");
    exit_button->setClickListener([](u64 keys) {
        if (keys & HidNpadButton_A) {
            tuneQuit();
            tsl::goBack();
            return true;
        }
        return false;
    });
    list->addItem(exit_button);

    /* About */
    list->addItem(new tsl::elm::CategoryHeader("О программе"));
    auto about_button = new tsl::elm::ListItem("О RyazhTune");
    about_button->setClickListener([frame](u64 keys) {
        if (keys & HidNpadButton_A) {
            const char *aboutLine1 = "RyazhTune v";
            char content[512];
            std::snprintf(content, sizeof(content),
                "Автор: Dimasick-git\n"
                "Создатель множества модулей и прошивки для Nintendo Switch\n\n"
                "Инструкция:\n"
                "- Плейлист: добавляйте треки вручную или через сканирование\n"
                "- Браузер музыки: выбирайте папки/файлы\n"
                "- Автовоспроизведение: включите при запуске\n"
                "- Белый список: включайте музыку только для выбранных игр\n"
                "- Загрузка игры: можно продолжать музыку во время загрузки");
            char header[128];
            std::snprintf(header, sizeof(header), "%s%s", aboutLine1, VERSION);
            frame->setToast(header, content);
            return true;
        }
        return false;
    });
    list->addItem(about_button);

    frame->setContent(list);

    return frame;
}

void MainGui::update() {
    static u8 tick = 0;
    /* Update status 4 times per second. */
    if ((tick % 15) == 0)
        this->m_status_bar->update();
    tick++;
}



