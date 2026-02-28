1. **Understand requirements**: Add update check functionality (`checkForUpdates` method in `PortageBackend`).
2. **Current state**: `PortageBackend::checkForUpdates()` is currently just a stub.
3. **What it should do**:
   - `checkForUpdates()` should execute `emergeSync()` (which is equivalent to `emerge --sync`).
   - We need to emit `fetchingUpdatesProgressChanged` properly.
   - We need to handle the progress returned from the `PortageAuthClient`.
   - On completion, it should recalculate the available updates (maybe reload the packages) and emit `updatesCountChanged()`.
