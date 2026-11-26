export default async function run(page, ui) {
  await page.locator('input[name="loginCode"], #loginCode').first().fill(process.env.ACS_USER).catch(async () => ui.fill('@e4', process.env.ACS_USER));
  await page.locator('input[name="password"], #password').first().fill(process.env.ACS_PW).catch(async () => ui.fill('@e5', process.env.ACS_PW));
  await page.getByRole('button', { name: 'Sign in' }).click();
  await page.waitForLoadState('networkidle').catch(() => {});
  await page.waitForTimeout(2000);
  return { url: page.url(), title: await page.title(), snap: await ui.snapshot() };
}
