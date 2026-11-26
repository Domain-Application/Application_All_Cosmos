export default async function run(page, ui) {
  return await page.evaluate(() => [...document.querySelectorAll('#edit-form [name], #edit-form domain-datepicker, #edit-form input')].map(e => e.tagName + ' name=' + (e.getAttribute('name') || '') + ' id=' + (e.id || '') + ' label=' + (e.getAttribute('label') || e.closest('[label]')?.getAttribute('label') || '')).slice(0, 40));
}
