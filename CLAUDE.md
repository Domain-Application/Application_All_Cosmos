# CLAUDE.md

Application_All_Cosmos: weighbridge program (VC6 / MFC, wlib32) for All Cosmos Industries Sdn Bhd with lane PCs (lane A / lane B), an office PC, LPR/CNR cameras (up to 10), platform sensors and header/detail tickets.

Module reference: [docs/MODULE_REFERENCE.md](docs/MODULE_REFERENCE.md) (Part 1 business view, Part 2 technical view, starts with "Differences from Application_Vance_Bioenergy").

**Rule:** Any change to behaviour, tables, fields, screens, reports or calculations must update the module reference (English, plus zh-CN when the changed section exists there) in the same commit.

Notes for working here:

- The real build is `menu.dsp` (34 `.cpp` files); 10 other `.cpp` files in the folder are not built (`MYCCTV.CPP` is built from this folder as a local copy of the library file).
- The menu is `IDR_MAINMENU` in `menu.rc` (25 items); every item is dispatched by `CMenuApp::SubProgram` in `MAIN.CPP`. Menu 31 (Set Password) is handled by the frame, `MAINFRM.CPP`.
- The exe's `ProductName` (`WB01`, `WB02`, or other) decides the role: lane A, lane B or office (`program4` 1, 2, 0).
- Tickets are header/detail pairs: `first`+`fdata`, `wb`+`wbdata`, `despatch`+`dbdata` (link `REFNO` = `TICKET`).
- The schema repository `SQL_All_Cosmos` is close to the code but its enums differ in places (see the module reference, code finding 8): check the live database before you trust a column order.
- This program has no capture reports, override links, bins, tankers or ReportEngine (the related Vance program does).
- Never put passwords or connection secrets into the docs; name the file and line instead.
