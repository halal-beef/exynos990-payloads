#include <stdint.h>
#include <stdbool.h>

#include <bootrom_funcs.h>

#include <epbl_checks.h>
#include <epbl_info.h>
#include <epbl_loader.h>

#include <exynos9830.h>

#include <pmu.h>

#include <memory.h>

int main(void)
{
	uint32_t ret = 0;

	usb_reinit(USB_STRUCT_ADDR, USB_DELAY, USB_SPEED_FULLSPEED);
	usb_send("Setting up last bits of BL1...");
	complete_bl1_rx_exec();
	usb_send("OpenMiniBL1 - Turning off SecureBoot...");
	writel(readl(0x02020070) & ~(1 << 2), 0x02020070);
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
	jump_to_epbl();

	return -1;
}
