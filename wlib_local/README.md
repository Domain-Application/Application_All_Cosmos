# wlib_local: project-only copies of three wlib32 files

These are copies of wlib32's `MODBUS.CPP`, `COMM.CPP` and `MYCCTV.CPP` (wlib32 commit `bedbdc4`,
26/11/2026) with thread and timer fixes. `menu.dsp` compiles them into this program. The linker then uses
these objects and does not take the same three files from `WLIB32.LIB`. **Only this program changes;** the
shared wlib32 and every other app stay as they are.

| File | Fix | Diff |
|---|---|---|
| `MODBUS.CPP` | `OnModbusTCP` always waits with a timeout (2 s connect, 5 s send/receive). Before, after 100 good polls it dropped the timeouts, and one lost PLC reply could hang the caller for good. `WSAStartup` is balanced, and there is no error box off the UI thread. | `MODBUS.CPP.diff` |
| `COMM.CPP` | Scale indicator thread: closing wakes the reader and closes the port; a second reader cannot start; TCP mode sleeps on a socket error instead of spinning at 100% CPU, frees the socket after a failed connect, and terminates its buffer. | `COMM.CPP.diff` |
| `MYCCTV.CPP` | `OnGetPlateNo` no longer loses its 3 MB of buffers on every failed plate read; `OnSnapshot` shows no box off the UI thread. | `MYCCTV.CPP.diff` |

The diffs (original wlib32 file vs. this copy) and the full write-up are in `wlib_local\*.diff` and in the
patch kit `C:\Domain\_work\wlib32_thread_patch\README.md` on AI-PC.

**When wlib32 changes one of these three files,** this copy does not follow: re-apply the `.diff` to the new
library version and copy it here. Otherwise this program keeps the old code. A link error `LNK2005`
(symbol defined twice) means the library file gained a function this copy lacks: refresh the copy.
