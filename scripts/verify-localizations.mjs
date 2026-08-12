#!/usr/bin/env node

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const localeDir = path.resolve(scriptDir, '../overlay/lang');
const stringsPath = path.resolve(scriptDir, '../overlay/source/strings.cpp');
const updateLocales = process.argv.includes('--update-locales') || process.argv.includes('--update-issue-38');
const updateBuiltinFallbacks = process.argv.includes('--update-builtin-fallbacks');
const removeLibraryStyle = process.argv.includes('--remove-library-style');

const issue38Translations = {
  'de.json': {
    WAIT_FOR_HOME: 'Auf HOME warten',
    PAUSE_ON_KEYBOARD: 'Bei Bildschirmtastatur pausieren',
    PAUSE_ON_CONTROLLER_SYNC: 'Bei Controller-Synchronisierung pausieren',
    PAUSE_ON_LOCKSCREEN: 'Am Sperrbildschirm pausieren',
  },
  'en.json': {
    WAIT_FOR_HOME: 'Wait For Home',
    PAUSE_ON_KEYBOARD: 'Pause On Keyboard',
    PAUSE_ON_CONTROLLER_SYNC: 'Pause On Controller Sync',
    PAUSE_ON_LOCKSCREEN: 'Pause On Lockscreen',
  },
  'es.json': {
    WAIT_FOR_HOME: 'Esperar a HOME',
    PAUSE_ON_KEYBOARD: 'Pausar con el teclado',
    PAUSE_ON_CONTROLLER_SYNC: 'Pausar durante la sincronización del mando',
    PAUSE_ON_LOCKSCREEN: 'Pausar en pantalla de bloqueo',
  },
  'fr.json': {
    WAIT_FOR_HOME: 'Attendre HOME',
    PAUSE_ON_KEYBOARD: 'Mettre en pause avec le clavier',
    PAUSE_ON_CONTROLLER_SYNC: 'Mettre en pause pendant la synchronisation de la manette',
    PAUSE_ON_LOCKSCREEN: 'Mettre en pause à l’écran verrouillé',
  },
  'it.json': {
    WAIT_FOR_HOME: 'Attendi HOME',
    PAUSE_ON_KEYBOARD: 'Pausa con la tastiera',
    PAUSE_ON_CONTROLLER_SYNC: 'Pausa durante la sincronizzazione del controller',
    PAUSE_ON_LOCKSCREEN: 'Pausa nella schermata di blocco',
  },
  'ja.json': {
    WAIT_FOR_HOME: 'HOMEまで待機',
    PAUSE_ON_KEYBOARD: 'キーボードで一時停止',
    PAUSE_ON_CONTROLLER_SYNC: 'コントローラー同期中に一時停止',
    PAUSE_ON_LOCKSCREEN: 'ロック画面で一時停止',
  },
  'ko.json': {
    WAIT_FOR_HOME: 'HOME까지 대기',
    PAUSE_ON_KEYBOARD: '키보드에서 일시 정지',
    PAUSE_ON_CONTROLLER_SYNC: '컨트롤러 동기화 중 일시 정지',
    PAUSE_ON_LOCKSCREEN: '잠금 화면에서 일시 정지',
  },
  'nl.json': {
    WAIT_FOR_HOME: 'Wachten op HOME',
    PAUSE_ON_KEYBOARD: 'Pauzeren bij toetsenbord',
    PAUSE_ON_CONTROLLER_SYNC: 'Pauzeren bij controllersynchronisatie',
    PAUSE_ON_LOCKSCREEN: 'Pauzeren op vergrendelscherm',
  },
  'pl.json': {
    WAIT_FOR_HOME: 'Czekaj na HOME',
    PAUSE_ON_KEYBOARD: 'Wstrzymaj przy klawiaturze',
    PAUSE_ON_CONTROLLER_SYNC: 'Wstrzymaj podczas synchronizacji kontrolera',
    PAUSE_ON_LOCKSCREEN: 'Wstrzymaj na ekranie blokady',
  },
  'pt.json': {
    WAIT_FOR_HOME: 'Aguardar HOME',
    PAUSE_ON_KEYBOARD: 'Pausar com teclado',
    PAUSE_ON_CONTROLLER_SYNC: 'Pausar durante a sincronização do controle',
    PAUSE_ON_LOCKSCREEN: 'Pausar na tela de bloqueio',
  },
  'ru.json': {
    WAIT_FOR_HOME: 'Ждать HOME',
    PAUSE_ON_KEYBOARD: 'Пауза на клавиатуре',
    PAUSE_ON_CONTROLLER_SYNC: 'Пауза при синхронизации контроллера',
    PAUSE_ON_LOCKSCREEN: 'Пауза на локскрине',
  },
  'uk.json': {
    WAIT_FOR_HOME: 'Чекати HOME',
    PAUSE_ON_KEYBOARD: 'Пауза на клавіатурі',
    PAUSE_ON_CONTROLLER_SYNC: 'Пауза під час синхронізації контролера',
    PAUSE_ON_LOCKSCREEN: 'Пауза на екрані блокування',
  },
  'zh.json': {
    WAIT_FOR_HOME: '等待 HOME',
    PAUSE_ON_KEYBOARD: '键盘时暂停',
    PAUSE_ON_CONTROLLER_SYNC: '控制器同步时暂停',
    PAUSE_ON_LOCKSCREEN: '锁屏时暂停',
  },
  'zh-cn.json': {
    WAIT_FOR_HOME: '等待 HOME',
    PAUSE_ON_KEYBOARD: '键盘时暂停',
    PAUSE_ON_CONTROLLER_SYNC: '控制器同步时暂停',
    PAUSE_ON_LOCKSCREEN: '锁屏时暂停',
  },
  'zh-tw.json': {
    WAIT_FOR_HOME: '等待 HOME',
    PAUSE_ON_KEYBOARD: '使用鍵盤時暫停',
    PAUSE_ON_CONTROLLER_SYNC: '控制器同步時暫停',
    PAUSE_ON_LOCKSCREEN: '鎖定畫面時暫停',
  },
};

const startupScreenTranslations = {
  'de.json': { STARTUP_SETTINGS: 'Starteinstellungen' },
  'en.json': { STARTUP_SETTINGS: 'Startup Settings' },
  'es.json': { STARTUP_SETTINGS: 'Ajustes de inicio' },
  'fr.json': { STARTUP_SETTINGS: 'Paramètres de démarrage' },
  'it.json': { STARTUP_SETTINGS: 'Impostazioni di avvio' },
  'ja.json': { STARTUP_SETTINGS: '起動設定' },
  'ko.json': { STARTUP_SETTINGS: '시작 설정' },
  'nl.json': { STARTUP_SETTINGS: 'Opstartinstellingen' },
  'pl.json': { STARTUP_SETTINGS: 'Ustawienia uruchamiania' },
  'pt.json': { STARTUP_SETTINGS: 'Configurações de inicialização' },
  'ru.json': { STARTUP_SETTINGS: 'Настройки запуска' },
  'uk.json': { STARTUP_SETTINGS: 'Налаштування запуску' },
  'zh.json': { STARTUP_SETTINGS: '启动设置' },
  'zh-cn.json': { STARTUP_SETTINGS: '启动设置' },
  'zh-tw.json': { STARTUP_SETTINGS: '啟動設定' },
};

const phraseKeys = {
  STARTUP_SETTINGS: 'Startup Settings',
  WAIT_FOR_HOME: 'Wait For Home',
  PAUSE_ON_KEYBOARD: 'Pause On Keyboard',
  PAUSE_ON_CONTROLLER_SYNC: 'Pause On Controller Sync',
  PAUSE_ON_LOCKSCREEN: 'Pause On Lockscreen',
};

const builtinLocaleTables = {
  'de.json': 'kDe',
  'es.json': 'kEs',
  'fr.json': 'kFr',
  'it.json': 'kIt',
  'ja.json': 'kJa',
  'ko.json': 'kKo',
  'nl.json': 'kNl',
  'pl.json': 'kPl',
  'pt.json': 'kPt',
  'uk.json': 'kUk',
  'zh.json': 'kZh',
  'zh-cn.json': 'kZhCn',
  'zh-tw.json': 'kZhTw',
};

const addAllTranslations = {
  'de.json': 'Alle hinzufügen',
  'es.json': 'Añadir todo',
  'fr.json': 'Tout ajouter',
  'it.json': 'Aggiungi tutto',
  'ja.json': 'すべて追加',
  'ko.json': '모두 추가',
  'nl.json': 'Alles toevoegen',
  'pl.json': 'Dodaj wszystko',
  'pt.json': 'Adicionar tudo',
  'uk.json': 'Додати все',
  'zh.json': '添加全部',
  'zh-cn.json': '添加全部',
  'zh-tw.json': '新增全部',
};

const files = fs.readdirSync(localeDir).filter((file) => file.endsWith('.json')).sort();
const expectedFiles = Object.keys(issue38Translations).sort();
let stringsSource = fs.readFileSync(stringsPath, 'utf8');
const pairsMatch = stringsSource.match(/kPairs = \{\{([\s\S]*?)\}\};/);
const failures = [];

if (!pairsMatch) {
  failures.push('Could not locate kPairs in overlay/source/strings.cpp');
}

const uiStringKeys = pairsMatch
  ? [...pairsMatch[1].matchAll(/\{\"((?:\\.|[^\"])*)\",\s*\"/g)].map((match) => JSON.parse(`\"${match[1]}\"`))
  : [];

if (uiStringKeys.length === 0) {
  failures.push('No UI translation keys were extracted from kPairs');
}

const issue38UiKeys = Object.values(phraseKeys);

function getBuiltinTable(source, tableName) {
  const pattern = new RegExp(
    `(constexpr std::array<const char \\*, static_cast<std::size_t>\\(Str::Count_\\)> ${tableName} = \\{\\{)([\\s\\S]*?)(\\n\\}\\};)`,
  );
  const match = source.match(pattern);
  if (!match) {
    throw new Error(`Could not locate built-in table ${tableName}`);
  }
  const entries = [...match[2].matchAll(/^\s*\"((?:\\.|[^\"])*)\",\s*$/gm)]
    .map((entry) => JSON.parse(`\"${entry[1]}\"`));
  return { match, entries };
}

function requiredTranslations(file) {
  return { ...issue38Translations[file], ...startupScreenTranslations[file] };
}

function fallbackOverrides(file) {
  const required = requiredTranslations(file);
  return {
    ...Object.fromEntries(Object.entries(phraseKeys).map(([id, phrase]) => [phrase, required[id]])),
    'Add All': addAllTranslations[file],
  };
}

if (removeLibraryStyle && pairsMatch) {
  const legacyIndex = uiStringKeys.indexOf('Startup Settings') + 1;
  for (const tableName of Object.values(builtinLocaleTables)) {
    const { match, entries } = getBuiltinTable(stringsSource, tableName);
    if (entries.length === uiStringKeys.length + 1) {
      entries.splice(legacyIndex, 1);
      const replacement = `${match[1]}\n${entries.map((entry) => `    ${JSON.stringify(entry)},`).join('\n')}${match[3]}`;
      stringsSource = stringsSource.replace(match[0], replacement);
      console.log(`removed library style fallback from ${tableName}`);
    }
  }
  fs.writeFileSync(stringsPath, stringsSource, 'utf8');
}

if (updateBuiltinFallbacks && pairsMatch) {
  for (const [file, tableName] of Object.entries(builtinLocaleTables)) {
    const { match, entries } = getBuiltinTable(stringsSource, tableName);
    const overrides = fallbackOverrides(file);
    let changed = false;
    for (const [key, value] of Object.entries(overrides)
      .sort(([left], [right]) => uiStringKeys.indexOf(left) - uiStringKeys.indexOf(right))) {
      const index = uiStringKeys.indexOf(key);
      if (index >= 0 && entries[index] !== value) {
        entries.splice(index, 0, value);
        changed = true;
      }
    }
    if (changed) {
      const replacement = `${match[1]}\n${entries.map((entry) => `    ${JSON.stringify(entry)},`).join('\n')}${match[3]}`;
      stringsSource = stringsSource.replace(match[0], replacement);
      console.log(`updated ${tableName}`);
    }
  }
  fs.writeFileSync(stringsPath, stringsSource, 'utf8');
}

for (const [file, tableName] of Object.entries(builtinLocaleTables)) {
  try {
    const { entries } = getBuiltinTable(stringsSource, tableName);
    if (entries.length !== uiStringKeys.length) {
      failures.push(`${tableName}: expected ${uiStringKeys.length} entries, found ${entries.length}`);
      continue;
    }
    for (const [key, value] of Object.entries(fallbackOverrides(file))) {
      const index = uiStringKeys.indexOf(key);
      if (index < 0 || entries[index] !== value) {
        failures.push(`${tableName}: wrong fallback for ${JSON.stringify(key)}`);
      }
    }
  } catch (error) {
    failures.push(error.message);
  }
}

if (JSON.stringify(files) !== JSON.stringify(expectedFiles)) {
  failures.push(`Unexpected language file set: ${files.join(', ')}`);
}

for (const file of files) {
  const filePath = path.join(localeDir, file);
  const contents = fs.readFileSync(filePath, 'utf8');
  let translations;
  try {
    translations = JSON.parse(contents);
  } catch (error) {
    failures.push(`${file}: invalid JSON (${error.message})`);
    continue;
  }

  if (removeLibraryStyle) {
    let removed = false;
    for (const key of ['SWITCH_2_STYLE', 'Switch 2 Style']) {
      if (Object.hasOwn(translations, key)) {
        delete translations[key];
        removed = true;
      }
    }
    if (removed) {
      fs.writeFileSync(filePath, `${JSON.stringify(translations, null, 4)}\n`, 'utf8');
      console.log(`removed library style entries from ${file}`);
    }
  }

  const required = requiredTranslations(file);
  if (!required) {
    failures.push(`${file}: no startup translation set`);
    continue;
  }

  for (const key of uiStringKeys) {
    if (typeof translations[key] !== 'string' || translations[key].trim().length === 0) {
      failures.push(`${file}: missing UI translation for ${JSON.stringify(key)}`);
    }
  }

  let changed = false;
  for (const [id, text] of Object.entries(required)) {
    const phrase = phraseKeys[id];
    for (const [key, value] of [[id, text], [phrase, text]]) {
      if (translations[key] !== value) {
        if (updateLocales) {
          translations[key] = value;
          changed = true;
        } else {
          failures.push(`${file}: ${key} must be ${JSON.stringify(value)}`);
        }
      }
    }
  }

  if (changed) {
    fs.writeFileSync(filePath, `${JSON.stringify(translations, null, 4)}\n`, 'utf8');
    console.log(`updated ${file}`);
  }
}

if (failures.length > 0) {
  console.error(failures.join('\n'));
  process.exit(1);
}

console.log(`Validated issue #38 translations in ${files.length} language files.`);
