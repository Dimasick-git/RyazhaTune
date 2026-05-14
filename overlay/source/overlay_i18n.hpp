#pragma once

/** Load Tesla / ultrahand JSON translations for the active config language. */
void reloadRyazhTuneTranslations();

/** One-shot toast after an overlay build/version bump (persists last seen in /config). */
void maybeShowOverlayWhatsNew();
