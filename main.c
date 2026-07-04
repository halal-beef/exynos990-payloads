#include <stdint.h>
#include <stdbool.h>

#include <bootrom_funcs.h>

#include <epbl_checks.h>
#include <epbl_info.h>
#include <epbl_loader.h>

#include <exynos9830.h>

#include <memory.h>

#include <pmu.h>

uint8_t usb_receive_hook(uint32_t rx_addr, uint32_t size)
{
	if(rx_addr == 0xBFE80000)
	{
		writel(0x6368, PTR_USB_RECEIVE);
		usb_receive(0xE8000000, 2.5 * 1024 * 1024);
		return usb_receive(rx_addr, size);
	}

	writel(0x6368, PTR_USB_RECEIVE);
	uint8_t ret = usb_receive(rx_addr, size);
	writel((uint32_t)(uintptr_t)usb_receive_hook, PTR_USB_RECEIVE);
	return ret;
}

int main(void)
{
	uint32_t ret = 0;

	usb_reinit(USB_STRUCT_ADDR, USB_DELAY, USB_SPEED_FULLSPEED);
	usb_send("Setting up last bits of BL1...");
	complete_bl1_rx_exec();
	usb_send("OpenMiniBL1 - Started");

	set_ps_hold();
	set_epbl_load_address();
	ret = load_epbl_usb();
	if(!ret)
		while(1);

	ret = verify_epbl_signature_and_rp_cnt(is_secure_boot());
	if(!ret)
		while(1);

	detect_and_patch_decrypted_epbl();

	set_status_bit(0, BL1_END);
	usb_send("OpenMiniBL1 - Attempting last minute patches...\n");
	writel((uint32_t)(uintptr_t)usb_receive_hook, PTR_USB_RECEIVE);
	usb_send("OpenMiniBL1 - Bye!");
	jump_to_epbl();

	return -1;
}
