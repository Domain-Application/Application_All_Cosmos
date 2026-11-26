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
| receiving_2nd_weigh.png | live, clip-5 run (S000164717) |
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
**NOT live - keep out of guides**: profile-screen fields for the API URL / user / password (set by IT in the DB);
`bin_location` (open point).
