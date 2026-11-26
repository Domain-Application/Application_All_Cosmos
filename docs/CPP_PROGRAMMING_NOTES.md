# C++ programming notes

## Conventions

- VC6 (VC98) MFC, ANSI `CString`, CRLF line endings.
- Columns added after the original layout are read with `GetDbFieldNo(df, "NAME")`, never by enum
  or position.

## VC98 toolchain gotchas

- A file-local helper named `Field` clashes with ADO's `Field` (from `msado60.tlh`): C2872
  "ambiguous symbol". Use another name.

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
- Timeouts: connect 10 s, send/receive 20 s. The call runs on the UI thread after the save.
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
  `AcsPickRefNo()`. Then one `AcsSendIn()` (FIRST) or `AcsSendOut()` (WB / DESPATCH).
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
