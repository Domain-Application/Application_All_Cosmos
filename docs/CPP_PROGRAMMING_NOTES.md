# C++ programming notes

## Conventions

- VC6 (VC98) MFC, ANSI `CString`, CRLF line endings.
- Columns added after the original layout are read with `GetDbFieldNo(df, "NAME")`, never by enum
  or position.
- **Threads never touch the screen or the database.** A worker thread posts a message to a window
  (`PostMessage` with a heap object the receiver deletes), and the UI thread does the screen and SQL work.
  If `PostMessage` fails (the window is gone), the sender deletes the object.
- **Stopping a thread:** use a stop flag or generation counter that the thread checks at least every 500 ms,
  wait a bounded time (3 s), and never `delete` a `CWinThread` that is still running. If it is still busy,
  set `m_bAutoDelete = TRUE` so MFC frees it when it ends, and leave allocated whatever the thread still reads.
- **Start threads suspended** (`CREATE_SUSPENDED`) when `m_bAutoDelete` must be changed first. A thread that
  ends at once could otherwise delete itself before the flag is set.
- **Shared data between threads** goes through one small lock: `CCriticalSection` (`<afxmt.h>`, e.g.
  `CMenuApp::m_csPlateNo`), or `CPlcLock` (`ADAM.CPP`) when a timeout is needed. **Do not create kernel
  handles (mutex, event) at static-init time** in this program: see `ADAM.CPP` below.
- These thread/timer rules were first proven in the sister app Application_Vance_Bioenergy (Vance) and
  ported here; the sections below say where AllCosmos differs.

## VC98 toolchain gotchas

- A file-local helper named `Field` clashes with ADO's `Field` (from `msado60.tlh`): C2872
  "ambiguous symbol". Use another name.
- `CWnd::ContinueModal()` is public in MFC42. After `EndDialog` it returns FALSE. Use it to avoid restarting
  a timer on a dialog that is closing.
- `CString` copies share one buffer by reference count. To hand a string to another thread, build a new
  `CString` from its `LPCTSTR`.
- VC98 declares `TryEnterCriticalSection` only for `_WIN32_WINNT >= 0x0400`, so `ADAM.CPP` declares it itself.

## `IT1.CPP` / `IT2.CPP` / `PMWeighing.cpp`: weight frozen at Save

The first-weight, second-weight and prime-mover screens rewrite the ticket weight from `cweight` every
250 ms (timer 1: `PGROSS`/`W1`/`Z1`, `Pm_WEIGHT`). `OnOK` can open message boxes before it ends the dialog:
the blank AllCosmos Ref No question (IT1), "Supplier weight is required for tanker" (IT2), the bin check and
`IsSaveConditionOk` (all three; `CReqitem::OnOK` for the prime mover). Those boxes pump messages, so the timer
kept running and the weight saved was not the one on screen when the operator answered (seen in Vance: test
ticket KR00036074 saved 8,190 kg after the check passed on 38,310 kg).

`OnOK` now calls `FreezeWeightTimer(this, TRUE)`, runs the old body as `DoOK()`, then
`FreezeWeightTimer(this, FALSE)`. That restarts timer 1 only while the dialog is still modal, so every early
`return` path unfreezes it. `FreezeWeightTimer` lives in `IT1.CPP` and is declared in `MENU.H`.

## `ADAM.CPP`: PLC polling thread

Before this change, `CSecurity::OnTimer` timer 3 (200 ms) called `readADAMData(adamData, 0)` inline on the
UI thread; in mode 2 (Modbus TCP, the Release default) that reads 24 coils (function 1) into `adamData`,
which `SensorStatus` and the sensor LEDs use. A Modbus read can block the UI for 2 s (connect timeout) or
more, and after 100 good polls the library's `OnModbusTCP` skips its `select` timeouts (`MODBUS.CPP`), so
one lost reply could freeze the screen for good.

- `PlcPollThreadProc` does the same read (`ReadPlc(adamData, 0)`) every 500 ms on its own thread. Timer 3
  now only redraws the sensor LEDs. The thread also polls while the security screen is closed (the old
  read ran only while it was open), so `adamData` is always current.
- **Coil writes go through the same thread.** While it runs, `WriteCoil` only queues the write
  (`QueueCoilWrite`, 256 entries, FIFO so "close coil off, open coil on" keeps its order; a write is skipped
  when the same coil already waits with the same value). The thread writes at most 16 queued writes per
  call, every 100 ms and once more as it stops. The weight lamp (`CNView::OnGw`, `weighbridge_ALT`), the
  automatic barrier opening (`InitiateDualMode` -> `CBarrier::OpenBarrier`) and the barrier buttons run from
  UI timers/clicks, so they never wait on the PLC. Before the thread starts, `WriteCoil` writes directly.
- It starts from `CNView::OnGw` (first weight tick) and **never in `OnInitialUpdate`**: in Vance, the view
  was initialised before login, while `ADAMSimulator` was still 0, and a thread started then went into the
  `ADAMTCP.dll` path. AllCosmos's `CMainFrame` has no one-shot start-up timer to start it from, and the
  view's weight timer can tick while the login box is up, so `StartPlcPolling` also waits until
  `CMainFrame::OnOpenOtherFiles` has set `ADAMSimulator` (1 or 2; 0 is never a real mode here).
- The thread does not read the weighbridge record. `StartPlcPolling` (and the direct `WriteCoil` path) set
  `szIPAddress` on the UI thread (`SetADAM_IPAdd`); `ReadPlc` and `WriteCoilNow` only use it. (Vance's
  `WriteCoilNow` still calls `SetADAM_IPAdd` on the thread; AllCosmos does not.)
- `plcLock` serialises reads and writes, so they do not mix the Modbus globals (`m_tid`,
  `_modbus_error_code`, `WriteCoilFlag`). A write waits at most 3 s; if a read is stuck in the library, the
  write goes ahead without the lock (the old behaviour).
- **`plcLock` is a critical section, not a `CMutex`.** In Vance a static `CMutex` (kernel handle created
  during start-up) was closed by other start-up code before the thread first used it; its waits then failed
  with `WAIT_FAILED` / error 5, because the handle number had been reused. A critical section has no handle,
  so it cannot be closed from outside.
- `PlcHasSignal()` is FALSE after 3 s with no good read (TRUE while no thread runs). In mode 2 a good read is
  `OnModbusTCP` TRUE and `_modbus_error_code == 0`; other modes always count as good.
  `CNView::ShowPlcSignal` logs the change ("PLC no reply for 3 s" / "PLC replies again") and sets status-bar
  pane 0.
- It stops in `CMainFrame::OnCloseThreads`, which `CMyFrame::OnClose` calls before `CloseAllDataFile`, and
  in `CNView::OnDestroy`.

## `BARRIER.CPP`: retry timer that never ran

`OnStartOpenTimer` / `OnStartCloseTimer` set timer IDs 3-6 on `AfxGetMainWnd()`. `CMainFrame` has no
`OnTimer` and `CMyFrame::OnTimer` does not pass those IDs on, so `CBarrier::OnTimer` (the 12-try retry and the
final clear of both coils) never ran. The timers just fired every 500 ms on the main frame for the life of
the program.

The `SetTimer` calls are removed; the bookkeeping (`TimerEvent`, `InProgress`) stays as it was. The guards
`TimerEvent==1` / `==2` in `OpenBarrier` / `CloseBarrier` are deliberately left unchanged. `TimerEvent` is
only reset inside the dead `OnTimer`, so a guard "fixed" to the real IDs (3-6) would stay true after the
first open, and every later open would be ignored. Making the retry loop work is a behaviour change for the
site (coil pulsing) and needs a test on the real barriers.

## `NVIEW.CPP` / `tcpServer.cpp`: ANPR listeners

AllCosmos runs two listeners, one per port field (`ANPR_PORT`, `ANPR_PORT2`), each with a `ServerInfo` in
`CNView::m_serverInstances`. The old code used blocking `CSocket` calls. `OnDestroy` and a port change
unblocked `Accept` with a "killer" connection, but a camera that stayed connected kept `Receive` blocked.
After the 5 s wait the code deleted the `CWinThread` and the `ServerInfo` while the thread still used them
(and the thread had published a pointer to its stack `CTcpServer` in `ServerInfo::pServer`).

- `CTcpServer` (same file as Vance) uses plain Winsock with a 500 ms `select` around `accept` and `recv`.
- The thread gets a `ServerThreadParam`: port, the view's `HWND`, its log file name and the listener's
  generation. It runs while `ServerInfo::generation` still has that value, and reads nothing else from the
  `ServerInfo`; the pointer only goes back to the UI thread in `wParam` of `WM_NEW_MESSAGE`, where
  `OnNewMessage` keeps the per-port buffer.
- `StopTCPServer(pInfo)` bumps that generation and waits up to 3 s. If the thread is still running, it gets
  `m_bAutoDelete = TRUE` and the `ServerInfo` is left allocated (`OnDestroy` does not delete it; a port
  change uses a new one).
- A port that cannot be opened is reported by posting `WM_ANPR_ERROR`, so the message box is shown by the
  UI thread. A failed `PostMessage` deletes the `CString`.

## `MENU.CPP` / `SECURITY.CPP`: photo thread

`PhotoThreadProc` (started by `CSecurity::OnTakePhotos`, from `OnInitDialog` and `IDC_BUTTON3`) runs the
camera calls off the UI thread.

- The push plates are already read and written under `CMenuApp::m_csPlateNo` (`Get/SetPushPlateno1/2`).
- `CMenuApp::OnGetPlateNo` writes only its `plateno` argument (no global), so it needed no change.
- The library's `OnLogsnapMsg` runs SQL (`LOG_SNAPSHOT`) and, in Debug, asserts. Called on the photo thread,
  `CMenuApp::OnLogsnapMsg` posts `WM_SNAPLOG` to the main frame, which calls it again on the UI thread
  (`CMainFrame::OnSnapLog`).
- `CSecurity::OnCancel` refuses to close for up to 10 s while `m_photoBusy`, so a second photo thread
  cannot start on the same cameras and files. The thread deletes its result string if the post fails.

## `SECURITY.CPP`: status timer

`CSecurity::OnGW` runs on every timer 3 tick (200 ms) and logged "Start Gw" / "End Gw" to the trace file
each time, about 10 lines a second. Those two lines are removed. `OnDestroy` kills timer 3 before its message
box.

## `AcsWeighbridgeApi.cpp` / `.h`

HTTP client for the AllcosmosSystem weighbridge API. Business view: `docs/ALLCOSMOS_API_INTEGRATION.md`.

- `AcsPost()` builds `<profile.API_URL>/api/<endpoint>`, writes an `AcsApiRequestLog` row, sends,
  then writes an `AcsApiResponseLog` row, whatever happened.
- **Basic header sent up front.** WinInet's user/password on `GetHttpConnection()` only answers a
  401 challenge. This site never sends one: a bad login is HTTP 200 with
  `{"isSuccess":false,"message":"Invalid username or password."}`. So the header is built here
  (`Base64(user:password)`).
- **Only `isSuccess` counts**, never the HTTP status. The scanner allows blanks around `:` because
  the site pretty-prints its JSON.
- Numbers: `%.3f` then trailing zeros and the dot trimmed (API: at most 3 decimals).
- Timeouts: connect 10 s, send/receive 60 s. The call runs on the UI thread after the save. Ticket Out and
  DO Time Out reply only after the website's own CSL call (GRN / Confirm DO, up to 30 s), so a shorter receive
  timeout gives "The operation timed out" although the website saved the data.
- Log tables are opened on first use and only if both exist (`sql/003`), so a site without the
  script never gets an "Invalid object name" error. `REQUEST_TS` / `RESPONSE_TS` are stamped in
  C++ because the ORM sends NULL, which bypasses a column DEFAULT.

## `AcsTicketMapping.cpp` / `.h`

Turns a FIRST / WB / DESPATCH row into the four calls. The API module knows nothing about tickets.

- `AcsIsActive()`: API URL set and `API_REF_NO` present on that table. Everything is a no-op
  otherwise, so the exe runs unchanged on a database without the new columns.
- `AcsIsDespatch()`: table DESPATCH, or FIRST with `[Print Counter]` = 2 (set in `OnFirstWt`).
- Dates are read back from SQL with `CONVERT(varchar(19), col, 120)`: `dat[]` may hold another
  format depending on `INIT_USE_SQLSERVER_TIME`.
- Flags are written to `dat[]` and straight to the row by `TICKET` (`SetTicketField`), because the
  row may already have been printed / moved and `SqlUpdate()` relies on the current record.
- Weights: `compute_weight()` fills `m[]` in kg. `m[11]` = nett after wastage (the app calls it the
  "theoretical weight", see `receive_save`), `m[5] − m[2]` = 2nd − 1st weight, `m[0]` = SWT.
- `AcsSendOut()` re-sends the 1st weigh first when `API_IN_OK` is not `Y`; Ticket Out needs it.
  If that fails, Ticket Out is flagged `N` without a second message box (the operator has just
  seen the Ticket In one).
- `SqlText()` throws on an empty result and `ComError` shows an error box, so every lookup here
  returns exactly one row (`isnull(min(...), '')`, `count(*)`).
- `AcsSendOut()` for a despatch ticket sends `actual_weight` = `m[11]` / 1000 with `actual_unit` `"MT"`: the
  website's Unit master has no KG.
- `AcsResendPending()` (Utility 39, `WM_COMMAND` 34503) lists the tickets flagged `N` in a
  `CAcsPickDlg` (FIRST / WB / DESPATCH union, newest first). The operator picks one;
  `TicketTable()` maps the Table column back to the `DB`. While `API_IN_OK` is not `Y`,
  `OfferNewRefNo()` asks "Pick a different one from the AllCosmos list?" and reuses
  `AcsPickRefNo()`. Then `AcsResendTicket()`.
- `AcsResendTicket()` is shared by Utility 39 and the ticket screens' Resend button: confirm when
  the flag is already `Y`, `OfferNewRefNo()` / `OfferBin()` when needed, then one `AcsSendIn()`
  (FIRST, by table name) or `AcsSendOut()` (WB / DESPATCH). A refused resend of an accepted
  ticket puts the flag back to `Y` (`Outcome()` always writes Y/N, but the website still holds
  the accepted data). The success box shows `s_sLastReply` (set in `Outcome()`), so the GRN /
  Confirm DO part of the reply is seen.
- `bin_location` comes from `API_BIN`, sent only when not blank. `AcsHasBin()` = active and
  sql/005 run; every bin control is hidden without it. `AcsPickBin()` lists `ACS_BIN_LOCATION`.
- `AcsAdminBinLocation()` (menu 294) opens `ACS_BIN_LOCATION` on first use, not in
  `OpenAllDataFile()`, because the table exists only after sql/005. It reuses `IDD_STATUS`
  (code + description, like Container / Trailer), which binds columns by position: CODE and
  DESCRIPTION must stay columns 1 and 2.
- `AcsStatusText()` builds the two-line status for `CWb`; `AcsShowCtrls()` shows/hides up to three
  controls (IT1, IT2, WB).
- `AcsPickRefNo()` (the "..." button) shows the view in a `CAcsPickDlg`, leaving out numbers
  already on a lorry in the yard (`FIRST.API_REF_NO`). Receiving returns the PO unit as the 2nd
  column.

## `AcsPickDlg.cpp` / `.h` (`IDD_ACS_PICK` 551, `IDC_ACS_LIST` 1286)

A plain list-view pick dialog over any SQL query (`m_title`, `m_sql`); returns the first two
columns of the chosen row in `m_pick1` / `m_pick2`. Built because wlib32's `DB::browse()` does
nothing in this app: `IDD_DbGrid` lives in wlib32's DBGRID.RC, which this app does not include,
so `DataGrid()` returns -1 silently. No OCX. Every column must come back as text (cast numbers
and dates in the SQL). Proven 2026-11-26 for the Ref No, DO No and resend lists.

## `IT1.CPP` (AllCosmos part)

- `IDC_ACS_REFNO` / `IDC_ACS_PICK` / `IDC_ACS_REFNO_LABEL` are not `IDC_EDITn`, so `CFEdit` does
  not bind them; `OnOK` copies the text to `API_REF_NO` by name. Hidden when `AcsIsActive()` is false.
- The PO unit from the pick list is kept only while the text still equals the picked Ref No.

## `IT2.CPP` (AllCosmos part)

- `AcsSendOut(pf)` runs after the FDATA lines are moved, i.e. after the ticket print, so a slow
  website never holds the printout. `pf->m_SaveFlag` is captured right after `AddNew()`.
- Consequence: when the Crystal job cannot open (`OpenPrintJob` returns NULL: missing `.rpt`, no
  Crystal runtime, the test-only CRPE32 stand-in), wlib32's `CPrintDlg::OnOK` (PRINTDLG.CPP)
  returns without `EndDialog`, and that dialog is hidden (`ShowWindow(SW_HIDE)` in
  `OnInitDialog`). `PrintOneFormOnly()` never returns, the screen stays on "Proceeding..." and
  `AcsSendOut()` is not reached. On a test PC, post `IDCANCEL` to the hidden "Printing now..."
  window; on a site, Resend covers the ticket.
- `IDC_ACS_REFNO` (read-only), `IDC_ACS_BIN` / `IDC_ACS_BIN_PICK` / `IDC_ACS_BIN_LABEL` (1287-1289)
  sit right of Container/Trailer. `AcsBinIsOk()` runs after `IsSaveConditionOk()` and writes
  `API_BIN` into `df` (FIRST); `OnSecond()`'s `fetch()` copies it to WB by name before `AddNew()`.

## `WB.CPP` (AllCosmos part)

`CWb` serves FIRST (menu 16), WB (11) and DESPATCH (12); `IDD_WB` and `IDD_DESPATCH` both carry
`IDC_ACS_STATUS` (1290) and `IDC_ACS_RESEND` (1291) right of Container/Trailer.
`ShowAcsStatus()` runs at the end of `ShowInformation()` (every record shown) and after a resend;
the button is disabled while no ticket is loaded.
`IDC_ACS_STATUS` holds exactly two lines (17 DLU) and has no room to grow (Container/Trailer on the
left, the weight group box on the right). So `AcsStatusText()` keeps both lines short ("Ref No: … Bin: …"
/ "1st: … 2nd: …", at most 37 characters on line 2) and the control is `SS_LEFTNOWORDWRAP`: a long
Ref No / bin is clipped at the right instead of wrapping and pushing the flags line out of the box.

## Testing on a PC without the COM loop cable

- An `ipadd` / `port` pair in win.ini `[Indicator]` wins over `device=` (wlib32 `SetCommThread`
  starts `ReadTCPThreadProc`), and the thread reads them once. A TCP stand-in that streams
  `+0018210` CR LF matches `leadch 43` + `XNNNNNNF`. `device=99` (`useTcp4demo`) is NOT enough
  while `ipadd` / `port` are set.
- win.ini for this unmanifested 32-bit exe is the per-user VirtualStore copy
  (`%LOCALAPPDATA%\VirtualStore\Windows\win.ini`), shared with every other wlib32 app run by the
  same user: back it up, change it only for the seconds the app needs to start, restore it.
- MFC modal dialogs in this app sometimes stay invisible on a hidden desktop (shown only on the
  first idle). A picked list then has to be found with `EnumDesktopWindows`.
