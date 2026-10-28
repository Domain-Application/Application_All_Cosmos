# C++ programming notes - Application_All_Cosmos

Code-level "why" for the C++ sources. The source files keep one-line remarks and point here
(`//Notes: docs/CPP_PROGRAMMING_NOTES.md.`). Business behaviour is in
[MODULE_REFERENCE.md](MODULE_REFERENCE.md).

## Conventions

- VC6 / MFC, ANSI build, CRLF line endings. Edit with tools that keep CRLF (Git Bash `sed -i`
  turns a file into LF).
- A field number is the column's position in the live table (`dat[n]`, `IDC_EDITn` on a
  CStaff/CReqitem dialog). Check the live database, not the schema repository, before
  binding a control.
- New control IDs go in `RESOURCE.H` with a value no shared header uses (`IDC_ACS_*`,
  1294-1299 for the AllcosmosSystem work).

## `AllcosmosWbApi.cpp` / `.h`

HTTP client for the four AllcosmosSystem weighbridge APIs (`/api/rawmaterialticketin`,
`rawmaterialticketout`, `dotimein`, `dotimeout`). It knows nothing about the ticket tables;
`AllcosmosTicketMapping.cpp` builds the JSON.

- **Config** is read from `PROFILE` by column name (`API_URL`, `API_USERNAME`,
  `API_PASSWORD`, `SITE_CODE`) through `ProfileText()`. A blank `API_URL` means the
  integration is off: nothing is sent and nothing is logged.
- **URL**: `API_URL` is the base address (e.g. `https://allcosmos.domainsb.com.my`); the code
  appends `/api/<name>`, and a trailing `/` on the base is removed first.
- **Auth**: the `Authorization: Basic ...` header is built by hand (`Base64()`) and sent with
  the first request. Passing the user and password to `GetHttpConnection()` instead makes
  WinInet wait for a 401 challenge, which fails against a server that does not send
  `WWW-Authenticate`.
- **Success** is HTTP 200 **and** `"isSuccess": true` in the body; the API answers HTTP 200
  with `isSuccess: false` for business errors. `message` is kept either way and shown on
  the ticket screen. `JsonValue()` is a small scanner for one key of a flat JSON object,
  not a JSON parser.
- **Timeouts** 10 s connect, 15 s send / receive, so a dead server cannot hold the lane
  screen for minutes. `CInternetException` is caught; the function never throws.
- **Logs**: one `AllcosmosApiRequestLog` row before the call (`TICKET_NO`, `API_NAME`, URL,
  JSON body; the password is never stored) and one `AllcosmosApiResponseLog` row after it,
  even when the call failed. `REQUEST_TS` / `RESPONSE_TS` are stamped in C++ because
  `AddNew()` sends blank columns as NULL, so the column `DEFAULT getdate()` never applies.
  `LogAllcosmosNotSent()` writes the same pair for a call that was refused before sending
  (site not set), so the reason shows on the ticket's status line.
- `HasAllcosmosSuccess()` answers "did this ticket's In call ever succeed?" from the logs;
  `GetAllcosmosStatusText()` formats the latest response row for the ticket screen.

## `AllcosmosTicketMapping.cpp` / `.h`

Which call a ticket needs and what goes in it.

| Call | When | Fields |
|---|---|---|
| Raw Material Ticket In | receiving, first weight (`IT1.CPP`) | `po_number` = line 1 `PO1`, `item_code` = header `PT`, `date_in` = `TIMEIN`, `site_code` = `PROFILE.SITE_CODE`, `lorry_no` = `LORRY`, `ticket_no` = `TICKET` |
| Raw Material Ticket Out | receiving, second weight (`IT2.CPP`) | `ticket_no`, `actual_weight` = net, `date_out` = `TIMEOUT` |
| DO Time In | despatch, first weight | `do_no` = line 1 `DO`, `date_in` |
| DO Time Out | despatch, second weight | `do_no`, `date_out`, `actual_weight` = net |

- **Line 1** is the detail line with the lowest `SQNO` in `fdata` / `wbdata` / `dbdata`,
  chosen by the header table name. A ticket without a PO No (receiving) or DO No
  (despatch) on line 1 is not sent: it is not an AllcosmosSystem delivery.
- **Dates** come from SQL Server (`convert(varchar(19), col, 120)`) for the saved row.
  `dat[]` holds a datetime in the PC's display format (e.g. `29/06/2026 10:37:33`), which
  the API rejects.
- **Net weight** is `compute_weight()`'s `m[11]` (first and second weight less the prime
  mover weights, less wastage), the same Nett the ticket screen shows, rounded to whole kg.
- **Out sends In first** when no In call for the ticket has succeeded (an office-prepared
  ticket, or an In that failed at the lane), because the API refuses an Out without an In.
  If that In fails, the Out is not sent.
- `DO_QTY` and `BIN_LOCATION` are **not** sent: AllcosmosSystem agreed to drop them from the
  API (2026-10-28). They are weighbridge-only fields.

## `IT1.CPP` (first weight)

- The PO No / DO No box (`IDC_ACS_PONO`) is unbound: the `FIRST` header has no such column.
  Its text is read in `CIt1::OnOK()`, and `AddFirstDetailLine()` writes it into `fdata`
  line 1 (`SQNO` 1, `PO1` or `DO`, and the header product) after the header is saved, so
  the line moves to `wbdata` / `dbdata` with the rest at the second weight.
- The box's length limit is the live column width (`fdata.fieldLength`): `DO` is only
  NVARCHAR(10) and `PO1` NVARCHAR(15).

## `IT2.CPP` (second weight)

`AllcosmosWeighOut(pf)` runs after the lines are moved to `wbdata` / `dbdata` and after the
ticket and D/O are printed, so a slow API never holds the printout.

## `WB.CPP` (ticket screen, menus 11, 12, 16)

- `ShowAllcosmosStatus()` shows the latest call's result and hides the status line and the
  **Resend to AllcosmosSystem** button when `API_URL` is blank. Resend is enabled only on a
  completed ticket (`WB` / `DESPATCH` with a time out) and calls the same
  `AllcosmosWeighOut()` as the lane.
- The detail grid columns come from `SetDetailGridTitles()` / `SetDetailGridRow()` in
  `WBDATA.CPP`, shared with the lane second-weight grid.

## `WBDATA.CPP`

- `SetDetailGridTitles()` / `SetDetailGridRow()`: the one list of detail-grid columns,
  used by `CWb` (office) and `CIt2` (lane). DO Qty (Kg) and Bin Location are the last two.
- `enum wbdata` (`MENU.H`) lists every column of `FDATA` / `WBDATA` / `DBDATA` up to
  `Wd_BIN_LOCATION` (25); `PKKEY` (26) is found by name. The old `Wd_PKKEY = 15` was wrong
  (position 15 is `GROSS_WEIGHT`) and unused, so it was removed.
- On `IDD_WBDATA` / `IDD_DBDATA`, DO Qty is `IDC_EDIT24` and Bin Location `IDC_EDIT25`
  (control ID = `IDC_EDIT1` + field number - 1).

## `PROFILE.CPP` (Company Profile, menu 35)

`SITE_CODE` is bound to a hidden `IDC_EDIT27`; the **Keluli** / **Suasa** radio buttons write
`KELULI` / `SUASA` into it and show the saved value. The radios change nothing while the box
is read-only or disabled (view mode). API URL, username and password are `IDC_EDIT24-26`.

## `MENU.CPP`

`menu.exe /testallcosmosapi K000000001 D000000001` opens the tables without the login,
sends each listed completed ticket's calls (K = receiving, D = despatch) and appends the
results to `apitest.log` in the working folder. Use it with a test database and a mock or
UAT endpoint, never against production.
