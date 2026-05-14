#include "strings.hpp"

#include "config/config.hpp"

#include <array>
#include <cstdio>
#include <cstring>

namespace i18n {

namespace {

char g_lang[8] = {};

struct Pair {
    const char *en;
    const char *ru;
};

// Indexed by underlying Str value — keep order identical to enum class Str.
// Count_ is a sentinel; it is NOT a row in this table.
constexpr std::array<Pair, static_cast<std::size_t>(Str::Count_)> kPairs = {{
    {"Player", "Плеер"},
    {"Settings", "Настройки"},
    {"Playlist", "Плейлист"},
    {"Browse", "Обзор"},
    {"Music Library", "Медиатека"},
    {"Volume", "Громкость"},
    {"Toggle Mute (Y)", "Без звука (Y)"},
    {"Music", "Музыка"},
    {"Game", "Игра"},
    {"Title ID", "Title ID"},
    {"Preset Volume", "Пресет громкости"},
    {"Default Focus", "Фокус по умолчанию"},
    {"Custom Focus", "Свой фокус"},
    {"Title Focus", "Фокус игры"},
    {"Home Focus", "Фокус HOME"},
    {"Miscellaneous", "Разное"},
    {"Playback Mode", "Режим воспроизведения"},
    {"Normal", "Обычный"},
    {"Whitelist", "Белый список"},
    {"Blacklist", "Чёрный список"},
    {"Whitelist", "Белый список"},
    {"Blacklist", "Чёрный список"},
    {"Language", "Язык"},
    {"Auto-play Startup", "Автовоспроизведение при запуске"},
    {"Remove Startup", "Сбросить автозапуск"},
    {"Stop RyazhTune", "Остановить RyazhTune"},
    {"On", "Вкл"},
    {"Off", "Выкл"},
    {"Pass", "Нет"},
    {"Play", "Играть"},
    {"Pause", "Пауза"},
    {"Language", "Язык"},
    {"Empty...", "Пусто..."},
    {"Tracks", "Треки"},
    {"Playlist", "Плейлист"},
    {"Playlist is empty!", "Плейлист пуст!"},
    {"Couldn't open: ", "Не удалось открыть: "},
    {"Add To Playlist", "В плейлист"},
    {"Set As Startup", "Автозапуск"},
    {"Stopped Scanning Folder", "Сканирование остановлено"},
    {"Too many entries in folder.", "Слишком много элементов в папке."},
    {"Failed to switch to folder.", "Не удалось переключить папку."},
    {"Added 1 track to Playlist.", "Добавлен 1 трек в плейлист."},
    {"Failed to add track.", "Не удалось добавить трек."},
    {"Startup File Set", "Файл автозапуска задан"},
    {"Startup Folder Set", "Папка автозапуска задана"},
    {"Startup Path Removed", "Путь автозапуска сброшен"},
    {"No startup path set in config.", "Путь автозапуска не задан."},
    {"Something went wrong.", "Что-то пошло не так."},
    {"Unknown Artist", "Неизвестный исполнитель"},
    {"Interface updated — no restart needed.", "Интерфейс обновлён — перезапуск не нужен."},
    {"Open Browse and add tracks here.", "Откройте «Обзор» и добавьте треки сюда."},
    {"Y remove · X clear all · − startup", "Y — убрать · X — всё · − автозапуск"},
    {"Press + within a few seconds to undo.", "Нажмите + в течение нескольких секунд для отмены."},
    {"Track restored to playlist.", "Трек возвращён в плейлист."},
    {"Tip: Settings has playback modes, Home Focus, and language.", "Подсказка: в «Настройках» — режимы, фокус HOME и язык."},
}};

static_assert(kPairs.size() == static_cast<std::size_t>(Str::Count_),
              "kPairs size must match Str::Count_");

constexpr std::size_t idx(Str id) {
    return static_cast<std::size_t>(id);
}

bool isRussian() {
    return std::strcmp(g_lang, "ru") == 0;
}

} // namespace

void syncFromConfig() {
    config::get_language(g_lang, sizeof(g_lang));
}

bool isRu() {
    return isRussian();
}

const char *t(Str id) {
    const auto i = idx(id);
    if (i >= kPairs.size())
        return "";
    return isRussian() ? kPairs[i].ru : kPairs[i].en;
}

const char *trackCountLabel(std::uint32_t count) {
    static char buf[48];
    if (count == 1u) {
        if (isRussian())
            return "1 трек";
        return "1 track";
    }
    if (isRussian()) {
        /* Russian plural rules for "трек". */
        const unsigned n     = static_cast<unsigned>(count);
        const unsigned n10   = n % 10u;
        const unsigned n100  = n % 100u;
        const char *word     = "треков";
        if (n10 == 1u && n100 != 11u)
            word = "трек";
        else if (n10 >= 2u && n10 <= 4u && (n100 < 10u || n100 >= 20u))
            word = "трека";
        std::snprintf(buf, sizeof(buf), "%u %s", n, word);
    } else {
        std::snprintf(buf, sizeof(buf), "%u tracks", count);
    }
    return buf;
}

} // namespace i18n
