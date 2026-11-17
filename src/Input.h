#pragma once

namespace BuffHUD::Input
{
	bool Init();

	// True while the panel should be on screen: either pinned by a short tap,
	// or the key is currently held past the configured threshold.
	[[nodiscard]] bool PanelVisible();

	// Re-evaluates the hold timer. Must run once per rendered frame; it does
	// nothing but compare two timestamps.
	void Tick();

	// One-shot capture of the next keyboard key, used by the settings page's
	// key-binding rows. Implemented here because this is the translation unit
	// that owns the SKSE input sink; the settings page just calls these.
	void BeginCapture();
	[[nodiscard]] bool IsCapturing();
	// Returns the captured DirectInput scan code (>= 0) once, then -1 afterwards.
	[[nodiscard]] int ConsumeCapturedKey();
}
