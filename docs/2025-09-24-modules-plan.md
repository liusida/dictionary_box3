## Dictionary_v2 modularization plan

Date: 2025-09-24

### Goals
- Extract cohesive modules into `./lib/` to enable isolated unit testing and clearer ownership.
- Maintain a thin application layer that composes drivers, business logic, and screens.

### Planned modules

- **core-eventing**
  - Purpose: Decoupled communication and lifecycle contracts.
  - Contents: `events`, `event_system`, `event_publisher`, `driver_interface`.
  - Used by: all other modules.

- **drivers-display**
  - Purpose: Display, touch and LVGL integration lifecycle.
  - Contents: `display_manager` (+ LVGL callbacks), depends on `TFT_eSPI`, `GT911`, `lvgl`.
  - Used by: app, screens.

- **drivers-wifi**
  - Purpose: WiFi connection management, credential persistence, simple HTTPS helper.
  - Contents: `wifi_control` (Preferences, scan/connect/status, POST helper).
  - Used by: app, business, screens.

- **drivers-ble-keyboard**
  - Purpose: BLE scanning/connection and key notification pipeline.
  - Contents: `ble_keyboard` (NimBLE wrapper, discovery, connect, key callbacks).
  - Used by: app, input, screens.

- **drivers-audio**
  - Purpose: HTTP MP3 streaming and audio pipeline management.
  - Contents: `audio_manager` (AudioTools pipeline, play/stop/volume, FreeRTOS task).
  - Used by: business, screens, app.

- **business-dictionary**
  - Purpose: Dictionary API client and audio orchestration.
  - Contents: `dictionary_api` (lookup, url encoding, base url, audio endpoints).
  - Depends on: `drivers-audio`, WiFi readiness.

- **input-key-processor**
  - Purpose: Converts BLE keyboard input into UI text and function actions via events.
  - Contents: `key_processor` (event buses, function queue, LVGL forwarding).
  - Depends on: `core-eventing`.

- **screens-splash**
  - Purpose: Splash flow, connectivity bootstrap, and transitions.
  - Contents: `splash_screen` (timers, connectivity init, status icons).
  - Depends on: `drivers-wifi`, `drivers-ble-keyboard`, LVGL symbols.

- **screens-main**
  - Purpose: Main screen controller for word lookup and audio.
  - Contents: `main_screen` (submit handling, dictionary API integration, status icons, key callbacks).
  - Depends on: `business-dictionary`, `drivers-wifi`, `drivers-audio`, `drivers-ble-keyboard`, LVGL symbols.

- **screens-wifi-settings**
  - Purpose: WiFi settings controller.
  - Contents: `wifi_settings_screen` (scan/connect/save flows, status icons).
  - Depends on: `drivers-wifi`, LVGL symbols.

- (optional) **app-core**
  - Purpose: High-level application orchestration and unified state manager.
  - Contents: `app`, `state_manager` (state transitions, recovery cooldowns).
  - Depends on: all drivers + screens, `core-eventing`.

### Testing scope per module (high level)
- core-eventing: EventBus subscribe/publish/unsubscribe, queued processing order, publisher routing.
- drivers-display: lifecycle readiness, LVGL callback wiring (with fakes), touch handling state changes.
- drivers-wifi: credentials save/load/clear, connection state machine timing, scan results.
- drivers-ble-keyboard: keycode→char mapping, discovery list management, callback dispatch.
- drivers-audio: play/stop state transitions, URL persistence, volume changes, task start/stop.
- business-dictionary: urlEncode, base URL handling, audio URL construction, driver invocation via mock.
- input-key-processor: function queue behavior, event emission and handling, LVGL forwarding.
- screens-* controllers: controller logic only using fakes/stubs for drivers and LVGL symbols.
- app-core: state transitions and recovery logic with mocked drivers/screens.

### Extraction order (recommendation)
1) core-eventing
2) drivers-* (display, wifi, audio, ble)
3) business-dictionary, input-key-processor
4) screens-* controllers
5) app-core (optional)

### Post-extraction notes
- Keep LVGL-generated C files (`src/ui/*`) as app assets; controllers consume their symbols.
- After moving to `lib/`, update includes to library-form and add `library.json` stubs per module.
- Minimize cross-dependencies; prefer depending on `core-eventing` and `driver_interface` where possible.


