# AllCosmos weighbridge API - handoff (paused 2026-11-26 22:45 on KELVIN)

Code: GitHub `Domain-Application/Application_All_Cosmos`, branch `feature/allcosmos-weighbridge-api`.
Spec (James's guide, Postman collection, email details incl. the UAT login): `docs/allcosmos-spec/`. Test scripts: `tools/acs-test-kit/` (paths inside point at KELVIN's scratchpad - adjust). Videos and website
screenshots are not in git: `\\kelvin\Dev Container\AllCosmos-API-handoff\`.

## Prompt to paste into Claude Code on the other PC

> Continue the AllcosmosSystem weighbridge-API test work Kelvin started on KELVIN. Read
> `\\kelvin\Dev Container\AllCosmos-API-handoff\HANDOFF.md` first, all of it, then do the "Still to do"
> list in order. The code is in `\\kelvin\Dev Container\Git Repo\Application\Weighing\Application_All_Cosmos`
> (uncommitted changes on `main` - do not commit unless Kelvin asks). Project facts: the repo's
> `.claude\skills\all-cosmos` skill (SKILL.md + learned.md). Use the `wlib32-app-run-on-new-pc`,
> `browser-automation`, `vc6-build-debug` and `mssql-mcp` skills. Turn Fortinet VPN off (torry is
> unreachable with it on).

## What is done

- Website test data made on UAT (https://allcosmos.domainsb.com.my/, login DOMAIN, password in `docs/allcosmos-spec/README.md`) through API Demo + the normal pages:
  - PO `WB-PO-0001` (RM-MOP60E, MT) -> Raw Material Planning key 2: Ref `WB-PO-0001-RM-MOP60E-1` (done:
    in+out, ticket S000164714) and `-2` (Ticket In only: lorry ACST202, ticket S000164715, still in the yard).
  - PO `WB-PO-0002` (RM-RPH26, MT) -> planning key 3: `WB-PO-0002-RM-RPH26-1` (done: in+out, ticket
    S000164716, lorry ACST301) and `-2` (still open - use it for clip 5).
  - Despatch Ticket `WB-DT-0001` / DO `WB-DO-0001` (done, lorry WA8328) and `WB-DT-0002` / DO `900260001`
    (done, lorry CCH4092). DDP keys 5 and 6, Delivery Date 2026-11-26 (only "today" DOs show in the list -
    on another day make a new Despatch Ticket + DDP with `kit\web\make-data.mjs`).
- Weighbridge test DB `allcosmos suasa` on torry (SQL 2008 R2, compat 80): sql/001-003 run, profile set to
  UAT, sql/900 (TEST-ONLY stand-in for the website sync: 11 tables, James's names) + sql/004 views run.
  `kit\sync-sim1.sql` = what the real sync would copy back (ticket/Time In/Out) so finished lines leave the
  pick list. **Set 2 is not yet synced back**: run an equivalent update for `WB-PO-0002-RM-RPH26-1`
  (S000164716, ACST301, in 2026-11-26 22:32:42, out 22:33:43) and DDP 6 (in 22:36:07, out 22:39:09).
- Code changes since the first report (all built, Debug + Release earlier; Debug rebuilt last):
  - New `AcsPickDlg.cpp/.h` + `IDD_ACS_PICK` (551) / `IDC_ACS_LIST` (1286): list-view pick dialog over a
    SQL query (no OCX). wlib32 `DB::browse()` never worked here (`IDD_DbGrid` lives in DBGRID.RC, not in this app).
  - "..." pick list uses it; hides Ref/DO Nos already on a lorry in the yard (FIRST).
  - Resend (Utility 39 = WM_COMMAND 34503): list of tickets flagged N -> pick one -> "pick a different Ref
    No / DO No?" -> resend that one ticket.
  - DO Time Out sends `actual_unit` **MT** (kg / 1000): UAT Unit master has no KG.
  - Ticket Out when the 1st weigh failed: one message, not two. SqlText lookups always return one row.
- Videos (5 fps frames -> mp4) in `video\`: 1 Ticket In, 2 Ticket Out, 3 DO Time In, 4 DO Time Out - all
  accepted by UAT. Right panel = request JSON + website reply + ticket flags, read live from the log tables.
  `website-proof\` = UAT page screenshots (planning line after in/out, Lorry Que after DO Time In).

## Still to do

1. **Clip 5 - failure + resend** (Kelvin asked for "all the api changes"): lorry e.g. ACST302, receiving
   1st weigh, TYPE a wrong Ref No (e.g. `WB-PO-0002-RPH26-2`) -> website rejects, ticket saved with
   API_IN_OK=N, message shown -> Utility 39 -> pick that ticket -> Yes -> pick `WB-PO-0002-RM-RPH26-2`
   -> "accepted" message. Then its 2nd weigh (Ticket Out). Note: the resend list also shows two old test
   tickets from the first session (WB S000164713 "TEST-002", DESPATCH C000002404 "TEST-DO-001") - pick
   the right row.
2. Website proof screenshots for clips 3-4: `DailyDeliveryPlanning/Edit?modelKey=6` (Time In/Out, Actual
   Qty 20.02 MT) and Lorry Que (CCH 4092 Completed) - `kit\web\shot-table.mjs` / `shot-page.mjs`.
3. Assemble the final videos: for each clip append the website screenshot (5-6 s still, same caption bar
   style) and a title card; one mp4 per requirement + one combined. ffmpeg (Gyan.FFmpeg via winget on KELVIN).
4. Update `docs\ALLCOSMOS_API_INTEGRATION.md` and `docs\CPP_PROGRAMMING_NOTES.md` for: CAcsPickDlg, new
   Resend flow, MT for DO Time Out, sql/900 (test only), the findings below. Rebuild Release (`.vscode\vc6-build.ps1 -Config Release -Rebuild`).
5. Final report to Kelvin (inline SQL per mssql-mcp rules; which DB; what was written by Claude on torry:
   sql/001-004 + 900 + small test updates; cleanup script for test rows/tables).

## Findings to report

- **CSL GRN needs `bin_location`**: every Ticket Out reply says "Posting the GRN to CSL failed: ... Bin
  Location can't be blank". The guide calls it optional; production GRNs will fail without it. Needs a
  decision (a fixed bin per site in profile? a field on the 2nd-weigh screen?) - ask Kelvin/James.
- **CSL Confirm DO needs numeric DO No** (max 9 digits) - test data only; real DOs are numeric.
- UAT Unit master has no `KG` -> the app now sends MT for DO Time Out.
- The app stores lorry plates without spaces (CCH 4092 -> CCH4092) but the 2nd-weigh search uses the typed
  text: typing "CCH 4092" opens a NEW 1st weigh. Existing behaviour, not from this change.
- Supervisor approval (CLogerr, LOGERR.CPP:74) compares the typed password with user.ACCESS as plain text;
  U099's login password is refused there. Keep Supplier Wt <= nett - wastage in tests.
- The API Demo page embeds an API credential (`apiCredential = "SYSDEV:..."`) in its HTML - tell James.

## How the test rig works (KELVIN; adapt paths on the other PC)

- Test copy of the app: Debug `menu.exe` + repo root files in a run folder, `PROVIDER.SYS` line 1 =
  `kit\PROVIDER.test-copy.SYS` (Data Source=192.168.0.10,1433; Initial Catalog=allcosmos suasa) - the app's
  SQLOLEDB can't resolve the name `torry`.
- Hidden desktop: `launch-hidden.ps1` + `gui-server.ps1` (skill wlib32-app-run-on-new-pc); `kit\hdc.ps1`
  has StartAndLogin (U099), Cap, StartRec/StopRec, Wt. Login U099 (password from Kelvin, not stored here).
- Weight: win.ini `[indicator] device=4,...` (COM4). On KELVIN a COM5->COM4 cable + indicator-standin on
  COM5 reading `%TEMP%\indicator-weight.txt` (restore 18210 when done). Another PC needs its own stand-in
  (skill wlib32-indicator-mask) - check its win.ini/COM ports first. Weighing Control needs Zero Detected
  (weight 0 for ~5 s first), Stable, >= 1000 kg.
- Wait for each window (WaitWin) before clicking; close Crystal previews before the next F5.
- DB checks: `kit\q.ps1` (SqlClient by IP, reads the login from the run folder's PROVIDER.SYS).
- Browser: `browser.mjs --session acs` (log in with `kit\web\login.mjs`, env ACS_USER/ACS_PW); the site's
  own JS must run in the main world - `kit\web\mainworld.mjs` injects a script tag.
