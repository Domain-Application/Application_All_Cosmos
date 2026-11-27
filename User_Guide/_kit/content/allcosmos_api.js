// AllCosmos weighbridge <-> AllcosmosSystem link: one guide for operators, supervisors and support.
// Figures are file names in the work folder's hq/ folder (copied from ../screens).
const { makeBuilder } = require('./build');

module.exports = function guide() {
  const b = makeBuilder();

  b.H1('1. About This Guide');
  b.P('The weighbridge program now reports every weighing to the AllcosmosSystem website. This guide shows the weighbridge operator what is new on the weighing screens, what to do when the website refuses a ticket, and how support staff set up and look after the link.');
  b.P('Chapters 3 to 8 are for operators and supervisors. Chapters 9 and 10 are for support / IT staff.');
  b.H2('Words used in this guide');
  b.table(['Word', 'Meaning'], [
    ['AllcosmosSystem (the website)', 'The company website where the planning team keeps Purchase Orders, Raw Material Planning, Import Schedule & Costing, Despatch Tickets and Daily Delivery Planning.'],
    ['Ref No', 'The number of one delivery line on the website (Raw Material Planning or Import Schedule & Costing), for example WB-PO-0002-RM-RPH26-2. Used for receiving.'],
    ['DO No', 'The Delivery Order number of a Daily Delivery Planning line. Used for despatch.'],
    ['Ticket In / Ticket Out', 'What the weighbridge sends to the website after the 1st and the 2nd weigh of a receiving lorry.'],
    ['DO Time In / DO Time Out', 'The same for a despatch lorry.'],
    ['Accepted / refused', 'The website answers every message. Accepted = the website saved it. Refused = it did not; the website says why.'],
    ['Resend', 'Sending a refused ticket again, after fixing the problem.'],
    ['Bin Location', 'The store bin in CSL where the received goods go. CSL needs it to post the Goods Receipt (GRN). Picked at the receiving 2nd weigh.'],
  ], [3, 7]);
  b.H2('Signs used in this guide');
  b.callout('tip', 'A shortcut or good practice that saves time.');
  b.callout('note', 'Extra information that helps you understand the screen.');
  b.callout('warn', 'Something you must do, or the link will not work.');

  b.H1('2. The Process at a Glance');
  b.flow([
    ['Planning team', 'Plan the delivery on the website', 'Raw Material Planning / Import Schedule & Costing (receiving) or Daily Delivery Planning (despatch).'],
    ['Operator', '1st weigh', 'Pick the Ref No or DO No on the 1st-weigh screen and Save. The program sends Ticket In / DO Time In (Chapters 4 and 6).'],
    ['Operator', '2nd weigh', 'Weigh out as usual and Save. The program sends Ticket Out / DO Time Out with the weights (Chapters 5 and 6).'],
    ['Website', 'Updates the planning line', 'Ticket number, lorry, time in / out and quantities appear on the website (Chapter 8).'],
    ['Operator', 'Only if refused: Resend', 'Fix the Ref No / DO No and send again (Chapter 7).'],
  ]);
  b.callout('note', 'The weighing is always saved first. A slow or refusing website never stops the lorry; a refused ticket is simply marked for Resend.');

  b.H1('3. Where to Find the Screens');
  b.table(['Screen', 'How to open it'], [
    ['1st-weigh and 2nd-weigh screens', 'Main screen > **F5-> Weighing Process** > Weighing Control: choose **Receiving** or **Despatch**, key in the lorry plate, press **F5-> Confirm Weight**. A lorry not in the yard gets the 1st-weigh screen (1/2); a lorry in the yard gets the 2nd-weigh screen (2/2).'],
    ['Resend list', 'Menu **3. Utility** > **39 AllCosmos Resend Pending Tickets**.'],
    ['AllCosmos status of one ticket, and its Resend button', 'Menu **1. Weighing** > **11 Receiving Weighing**, **12 Despatch Weighing** (completed tickets) or **16 First Weighing** (lorries in the yard). Open the ticket.'],
    ['Bin Location list (support)', 'Menu **2. Table** > **29 More Master Setup** > **294 Set AllCosmos Bin Location**.'],
  ], [3.2, 6.8]);
  b.callout('tip', 'Key in the lorry plate without spaces (CCH4092, not CCH 4092). The program keeps plates without spaces; with a space it does not find the lorry in the yard and opens a new 1st weigh.');

  b.H1('4. Receiving - 1st Weigh');
  b.P('The 1st-weigh screen has a new box, **AllCosmos Ref No.**, with a **...** button next to it.');
  b.steps([
    'Fill the screen as before (supplier, product, driver and so on).',
    'Press **...** next to **AllCosmos Ref No.** The list of planned delivery lines opens.',
    'Click the line this lorry is delivering and press **OK**. The Ref No is filled in.',
    'Press **Save**.',
  ]);
  b.fig('receiving_1st_weigh.png', 'Receiving 1st-weigh screen with the new AllCosmos Ref No. box');
  b.fig('ref_pick_list.png', 'AllCosmos Ref No. list (Raw Material Planning / Import Schedule & Costing)');
  b.callout('note', ['The list shows only lines that are not already on a lorry in the yard.', 'You may also type the Ref No instead of picking it, but a typing mistake means the website refuses the ticket (Chapter 7).']);
  b.callout('warn', 'If the box is blank, the program asks "Save anyway?". Answer No and pick the Ref No. Answer Yes only if you cannot find it now; the ticket is then sent later with Resend.');

  b.H1('5. Receiving - 2nd Weigh');
  b.P('The 2nd-weigh screen shows the lorry\'s **AllCosmos Ref No.** from the 1st weigh (it cannot be changed here) and a new **Bin Location** box.');
  b.steps([
    'Weigh the lorry out as usual and key in **Supplier Wt**.',
    'Press **...** next to **Bin Location**, click the bin where the goods are stored and press **OK**.',
    'Press **Save**. The program sends the received, weighbridge and supplier quantities and the bin to the website, in the unit of the Purchase Order (for example MT).',
  ]);
  b.fig('receiving_2nd_weigh.png', 'Receiving 2nd-weigh screen: AllCosmos Ref No. (read-only) and Bin Location');
  b.callout('warn', 'CSL cannot post the Goods Receipt without a bin. If the box is blank, the program asks "Save anyway?". Answer No and pick the bin. Answer Yes only if you cannot now; add the bin later with **Resend to AllCosmos** on the ticket (Chapter 7).');
  b.callout('note', 'If the bin you need is not in the list, ask support to add it (Chapter 10).');

  b.H1('6. Despatch - 1st and 2nd Weigh');
  b.P('On a despatch lorry the box is called **AllCosmos DO No.** It works the same way as the receiving Ref No.');
  b.steps([
    'Press **...** next to **AllCosmos DO No.**',
    'Pick the DO for this lorry and press **OK**, then **Save**. The program sends DO Time In.',
    'At the 2nd weigh, weigh out and **Save** as usual. The program sends DO Time Out with the nett weight in MT.',
  ]);
  b.fig('do_pick_list.png', 'AllCosmos DO No. list - only DOs with today\'s Delivery Date');
  b.fig('despatch_1st_weigh.png', 'Despatch 1st-weigh screen with the DO No filled in');
  b.callout('note', 'The DO list shows only Daily Delivery Planning lines whose Delivery Date is today. If the DO is not in the list, ask the planning team to check the Delivery Date on the website.');

  b.H1('7. When the Website Refuses a Ticket');
  b.P('If the website refuses a ticket, a message shows the ticket number and the website\'s reason. The weighing is already saved.');
  b.fig('msg_rejected.png', 'Example: the typed Ref No does not exist on the website');
  b.steps([
    'Press **OK** and let the lorry carry on as usual.',
    'Find out what is wrong (usually a wrong or missing Ref No / DO No).',
    'Open **3. Utility** > **39 AllCosmos Resend Pending Tickets**. The list shows every ticket the website has not accepted yet.',
    'Click the ticket and press **OK**.',
    'If the 1st weigh was refused, the program shows the Ref No / DO No it holds and asks "Pick a different one from the AllCosmos list?". Press **Yes**, pick the right line from the list (same list as in Chapter 4) and press **OK**.',
    'The ticket is sent again. "Ticket ... accepted by AllCosmos." means it is done; the ticket leaves the list.',
  ]);
  b.fig('resend_list.png', 'AllCosmos Resend Pending Tickets - tickets not accepted yet');
  b.fig('resend_ask.png', 'Change the Ref No before sending again?');
  b.fig('resend_ok.png', 'The website accepted the ticket');
  b.callout('note', ['In the list, **1st weigh OK** / **2nd weigh OK** show N for the part not accepted yet.', 'A lorry still in the yard can be resent too; its 2nd weigh is then sent normally.', 'The website accepts a resend until its Goods Receipt / Delivery Order confirmation is posted.']);
  b.callout('tip', 'Supervisors: open the Resend list once a day. It should be empty.');
  b.H2('Status and Resend on the ticket itself');
  b.P('Every ticket screen (11 Receiving Weighing, 12 Despatch Weighing, 16 First Weighing) shows, next to Container / Trailer, the ticket\'s Ref No / DO No, the bin, and whether the website accepted each weigh: **Accepted**, **NOT accepted** or **Not sent**.');
  b.steps([
    'Open the ticket.',
    'Read the AllCosmos line. To send the ticket again (for example after adding the bin or correcting a weight), press **Resend to AllCosmos**.',
    'If the ticket was already accepted, the program asks "Send it again?". Press **Yes**.',
    'If a receiving ticket has no bin, the program offers the bin list first.',
    'The reply shows the website\'s full answer, including the result of the Goods Receipt / Delivery Order confirmation in CSL.',
  ]);
  b.fig('ticket_status.png', 'Ticket screen: AllCosmos status and the Resend to AllCosmos button');
  b.fig('resend_again_ok.png', 'Website reply after Resend (here CSL still asks for a bin)');
  b.callout('note', 'Once CSL has posted the Goods Receipt or the delivery, the website refuses any change. The ticket then stays marked Accepted.');

  b.H1('8. Checking the Result on the Website');
  b.P('Anyone with a website login can check that a weighing arrived.');
  b.table(['Weighing', 'Where to look on the website', 'What you should see'], [
    ['Receiving', '**A - Receiving** > **A2 - Raw Material Planning** (or A3 - Import Schedule & Costing), open the PO line', 'In Delivery Details: Ticket No, the quantities and Time In / Time Out on the line of the Ref No.'],
    ['Despatch', '**B - Despatch** > **B3 - Lorry Que**', 'The lorry with Time In (status Filling) and, after the 2nd weigh, Time Out (status Completed). B2 - Daily Delivery Planning shows the Actual Quantity.'],
  ], [1.8, 4, 4.2]);
  b.fig('web_rmp_lines.png', 'Raw Material Planning - Delivery Details after two receiving lorries');
  b.fig('web_lorry_que.png', 'Lorry Que after a despatch lorry was weighed in and out');

  b.H1('9. Setting Up the Link (Support / IT)');
  b.P('Do this once per site, on the site database, after installing the new program version.');
  b.steps([
    'Install the new weighbridge program on every weighbridge PC.',
    'IT runs the AllCosmos set-up scripts supplied with the update (folder **sql**, scripts 001, 002 and 003) on the site database. They add the new fields and the message log; they do not change existing weighings.',
    'IT enters the three connection settings in the site\'s profile settings: the website address, the API user and the API password. The website team gives the user and password for the live site (the test site is https://allcosmos.domainsb.com.my/).',
    'Once the website data is copied into the site database (the data sync), IT runs script **004**. Until then the **...** button says the list is not available and operators type the Ref No / DO No.',
    'IT runs script **005** (Bin Location). Then support enters the site\'s CSL bin codes in **294 Set AllCosmos Bin Location** (Chapter 10). Ask the CSL user for the exact codes.',
    'Restart the weighbridge program.',
    'Test with one receiving and one despatch lorry and check the website (Chapter 8).',
  ]);
  b.callout('warn', ['Keep the API password out of e-mails, documents and shared folders.', 'The connection settings are not on a screen yet; only IT can change them.']);
  b.callout('note', 'Leaving the website address blank switches the link off. The program then works exactly as before, without the AllCosmos box.');

  b.H1('10. Looking After the Link (Support / IT)');
  b.table(['Situation', 'What to do'], [
    ['Every day', 'Check that **39 AllCosmos Resend Pending Tickets** is empty on every weighbridge. Resend or follow up what is left.'],
    ['The Ref No / DO list is empty', 'Check on the website that the planning line exists (for despatch: Delivery Date = today) and that the data sync to the site database is running. Operators can type the number meanwhile.'],
    ['Every ticket is refused with "Invalid username or password."', 'The API password was changed or has expired. IT enters the new one in the profile settings and restarts the program; then resend the tickets.'],
    ['"No answer from ..."', 'The weighbridge PC cannot reach the website (network, internet, website down). Check the connection, then resend.'],
    ['Website says the GRN to CSL failed: "Bin Location can\'t be blank"', 'The ticket was saved without a bin. Open it (11 Receiving Weighing), press **Resend to AllCosmos** and pick the bin.'],
    ['Website says the GRN to CSL failed for another reason (for example a bin or supplier CSL does not know)', 'The weighbridge part is done. The website team fixes the data and reposts the GRN on the website; nothing to do on the weighbridge.'],
    ['Add, change or remove a bin', '**2. Table** > **29 More Master Setup** > **294 Set AllCosmos Bin Location**: key in the CSL bin code (max 15 characters) and a description, then save. Remove a bin that is no longer used. The website has no bin list, so this list is kept on the weighbridge.'],
    ['A question about one ticket', 'Every message to and from the website is recorded in the program\'s AllCosmos log (time, ticket, what was sent, the website\'s reply). IT can read it.'],
    ['New site or new PC', 'Repeat Chapter 9 steps 1, 5 and 6 (the database steps are once per site).'],
  ], [3.4, 6.6]);
  b.fig('bin_master.png', '294 Set AllCosmos Bin Location: adding a bin');

  b.H1('11. Messages You May See');
  b.table(['Message', 'What it means / what to do'], [
    ['AllCosmos Ref No. / DO No. is blank. Save anyway? It can be added later with Utility > AllCosmos Resend Pending Tickets.', 'No Ref No / DO No on the 1st weigh. Answer No and pick it; Yes saves without it (send later with Resend).'],
    ['AllCosmos Ticket In, ticket ...: <reason>. The weighing is saved. Fix it and use Utility > AllCosmos Resend Pending Tickets.', 'The website refused the ticket. Same message for Ticket Out, DO Time In and DO Time Out. Fix the reason and resend (Chapter 7).'],
    ['... Ref No ... is not in Raw Material Planning or Import Schedule & Costing.', 'The Ref No does not exist on the website: picked wrongly or typed wrongly. Resend with the right one.'],
    ['... Ref No is blank. / ... DO No is blank.', 'The ticket was saved without a number. Resend and pick it.'],
    ['... PO unit \'...\' is not KG or MT; quantities cannot be converted.', 'The Purchase Order uses a unit the weighbridge cannot convert. Ask the planning team to check the PO unit.'],
    ['... No answer from ...', 'The website could not be reached. Resend later.'],
    ['The AllCosmos list (...) is not available yet. Type the Ref No. instead.', 'The website data is not in the site database yet (Chapter 9 step 4). Type the number.'],
    ['Ticket ..., lorry ...: Ref No is \'...\'. Pick a different one from the AllCosmos list?', 'Shown during Resend. Yes = choose another number; No = send with the same number.'],
    ['No AllCosmos tickets are waiting to be resent.', 'Nothing to do: every ticket was accepted.'],
    ['Ticket ... accepted by AllCosmos. <website reply>', 'The resend worked. Read the rest of the reply for the CSL result.'],
    ['Ticket ... is already accepted by AllCosmos. Send it again?', 'Shown by Resend to AllCosmos on an accepted ticket. Yes sends it again (for example after a correction).'],
    ['Bin Location is blank. CSL needs it for the GRN. Save anyway? ...', 'Receiving 2nd weigh without a bin. Answer No and pick the bin.'],
    ['Ticket ...: Bin Location is blank and CSL needs it for the GRN. Pick one now?', 'Shown during Resend. Yes = pick the bin before sending.'],
    ['The AllCosmos bin list (ACS_BIN_LOCATION) is not set up yet: run sql/005.', 'Script 005 has not been run on this site. Call support (Chapter 9).'],
    ['AllCosmos API is not set up: run sql/001-003 and fill in API URL, user and password in the profile table.', 'The link is switched off on this site. Call support (Chapter 9).'],
  ], [5, 5]);

  return b.build({ company: 'ALL COSMOS INDUSTRIES SDN BHD', system: 'Weighing System', version: '1.0',
    title: 'ALLCOSMOS LINK', subtitle: 'Weighbridge operators, supervisors and support',
    audience: 'Weighbridge operators, supervisors and support staff', header: 'ALLCOSMOS LINK - USER GUIDE' });
};
