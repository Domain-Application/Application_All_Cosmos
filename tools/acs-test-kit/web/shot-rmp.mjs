import { mainWorld } from './mainworld.mjs';
// Screenshot of Raw Material Planning (WB-PO-0001) delivery lines, scrolled to show Lorry / Ticket No / quantities / Time In-Out.
export default async function run(page, ui) {
  await page.setViewportSize({ width: 1920, height: 1080 });
  await page.goto('https://allcosmos.domainsb.com.my/RawMaterialPlanning/Edit?modelKey=2');
  await page.waitForTimeout(5000);
  const rows = await mainWorld(page, `return document.getElementById('sub-item-table').getDataList().map(r => ({ Lorry: r.LorryPlateNo, Ticket: r.TicketNo, Recv: r.ReceivedQuantity, WB: r.WeighbridgeQuantity, Sup: r.SupplierDeliveryQuantity, In: (r.TimeInDate||'') + ' ' + (r.TimeInTime||''), Out: (r.TimeOutDate||'') + ' ' + (r.TimeOutTime||'') }));`);
  await page.evaluate(() => document.querySelector('[data-collapsed], .domain-menu-collapse')?.click());
  await page.locator('#sub-item-table').scrollIntoViewIfNeeded();
  await page.waitForTimeout(800);
  await page.screenshot({ path: process.env.OUT || 'rmp-now.png' });
  return rows;
}
