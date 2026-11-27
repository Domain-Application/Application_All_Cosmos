# Guide kit: Application_All_Cosmos (AllCosmos link guide)

Lives in the repo at `User_Guide\_kit\`. Built with the `cpp-user-guide-builder` skill.

| Setting | Value |
|---|---|
| Repo | `Application_All_Cosmos`, branch `feature/allcosmos-weighbridge-api` |
| Exe | `Debug\menu.exe` (VC6, `.vscode\vc6-build.ps1`) |
| Test DB | `allcosmos suasa` on torry (`Data Source=192.168.0.10,1433` in the run copy's PROVIDER.SYS; the app cannot resolve the name `torry`). Test copy. |
| Login | U099 - password from Kelvin, never stored here |
| Output | `User_Guide\AllCosmos Link\ALLCOSMOS LINK USER GUIDE.docx` |
| Cover | company `ALL COSMOS INDUSTRIES SDN BHD`, system `Weighing System` |
| Content | `content\allcosmos_api.js` - one guide: operators (ch. 3-8) + support (ch. 9-10) + messages |
| Screens | `screens\*.png`, captured 2026-11-26/27 on AI-PC |

## Screens and where they came from

| File | Source |
|---|---|
| receiving_1st_weigh.png | live, hidden desktop, lorry GUIDE01, closed without saving (Exit > Yes) |
| receiving_2nd_weigh.png | live 2026-11-27, ACST202 2nd weigh (S000164715), cropped 809x555 to fit a landscape page |
| ticket_status.png | live 2026-11-27 (after the status-line fix), menu 11 with S000164719 loaded, crop 686x138 of the status line + Resend button: wider than 649, so it sits on a landscape page |
| resend_again_ok.png | live 2026-11-27, Resend to AllCosmos on S000164717 (already accepted) |
| bin_master.png | live 2026-11-27, menu 294 while adding BIN-TEST2 |
| ref_pick_list.png, resend_list.png, resend_ask.png, resend_ok.png | live, clip-5 run |
| despatch_1st_weigh.png, do_pick_list.png | cropped from Kelvin's clip 3 video frame (29.5 s / 37.5 s), app area only |
| msg_rejected.png | cropped from the clip-5 video (30 s) |
| web_rmp_lines.png, web_lorry_que.png | website (UAT) screenshots, 1920x1080, cropped |

## Recipes / quirks

- Weighing needs a weight feed: see `docs\CPP_PROGRAMMING_NOTES.md` "Testing on a PC without the COM loop cable".
- The pick dialog (`IDD_ACS_PICK`) sometimes stays invisible on a hidden desktop; find it with `EnumDesktopWindows`.
- The DO list shows only today's Delivery Date: capture it on the day the test DOs are dated.
- 2nd weigh: `settext` on Supplier Wt (2015) needs a Tab key (9) to that box before Save, or the app says
  "Supplier weight is required".
- Without Crystal, the 2nd-weigh print leaves a hidden "Printing now..." dialog: post IDCANCEL to it.

## Audit (2026-11-27)

**Live**: AllCosmos Ref No. / DO No. box + "..." (IT1), Utility 39 Resend, the messages in chapter 11 (texts from
AcsTicketMapping.cpp / IT1.CPP).
Live since 2026-11-27: Bin Location box + "..." on IT2, ticket-screen status line + Resend to AllCosmos (menus
11/12/16), menu 294 Set AllCosmos Bin Location.
**NOT live - keep out of guides**: profile-screen fields for the API URL / user / password (set by IT in the DB);
the printed ticket does not show the Ref No / bin.

Recipes added 2026-11-27: open a ticket screen with `menucmd` 34475/34476/34480, then type the ticket in 2001 +
Enter (edit mode; close without saving). Menu 294 = `menucmd` 34758. The bin pick list is hidden on the hidden
desktop: its first row is preselected, so post `WM_COMMAND 1` (IDOK) to it.
