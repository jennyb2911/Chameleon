#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "gma.h"
#include "nvidia.h"
#include "hda.h"
#include "modules.h"


extern bool setup_ati_devprop(pci_dt_t *ati_dev);
extern bool setup_nvidia_devprop(pci_dt_t *nvda_dev);
extern bool setup_gma_devprop(pci_dt_t *gma_dev);
extern bool setup_hda_devprop(pci_dt_t *hda_dev);
extern void set_eth_builtin(pci_dt_t *eth_dev);
extern void notify_usb_dev(pci_dt_t *pci_dev);
extern void force_enable_hpet(pci_dt_t *lpc_dev);

extern pci_dt_t *dram_controller_dev;

void setup_pci_devs(pci_dt_t *pci_dt)
{
	char *devicepath;
	bool doit, do_eth_devprop, do_gfx_devprop, do_enable_hpet, do_hda_devprop;
	pci_dt_t *current = pci_dt;

	do_eth_devprop = do_gfx_devprop = do_enable_hpet = do_hda_devprop = false;

	getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kGraphicsEnabler, &do_gfx_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kHDAEnabler, &do_hda_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kForceHPET, &do_enable_hpet, &bootInfo->chameleonConfig);

	while (current)
	{
		devicepath = get_pci_dev_path(current);

		switch (current->class_id)
		{
			case PCI_CLASS_BRIDGE_HOST:
				if (current->dev.addr == PCIADDR(0, 0, 0))
				{
					dram_controller_dev = current;
				}
				break;
				
			case PCI_CLASS_NETWORK_ETHERNET: 
				if (do_eth_devprop)
				{
					set_eth_builtin(current);
				}
				break;

			case PCI_CLASS_DISPLAY_VGA:
				if (do_gfx_devprop)
				{
					switch (current->vendor_id)
					{
						case PCI_VENDOR_ID_ATI:
							if (getBoolForKey(kSkipAtiGfx, &doit, &bootInfo->chameleonConfig) && doit)
							{
								verbose("Skip ATi/AMD gfx device!\n");
							}
							else
							{
								setup_ati_devprop(current);
							}
							break;

						case PCI_VENDOR_ID_INTEL:
							if (getBoolForKey(kSkipIntelGfx, &doit, &bootInfo->chameleonConfig) && doit)
							{
								verbose("Skip Intel gfx device!\n");
							}
							else
							{
								setup_gma_devprop(current);
							}
							break;

						case PCI_VENDOR_ID_NVIDIA:
							if (getBoolForKey(kSkipNvidiaGfx, &doit, &bootInfo->chameleonConfig) && doit)
							{
								verbose("Skip Nvidia gfx device!\n");
							}
								else
							{
								setup_nvidia_devprop(current);
							}
							break;
						}
					}
					break;

			case PCI_CLASS_MULTIMEDIA_AUDIO_DEV:
				if (do_hda_devprop)
				{
					setup_hda_devprop(current);
				}
				break;

			case PCI_CLASS_SERIAL_USB:
				notify_usb_dev(current);
				break;

			case PCI_CLASS_BRIDGE_ISA:
				if (do_enable_hpet)
				{
					force_enable_hpet(current);
				}
				break;
		}
		
		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		
		setup_pci_devs(current->children);
		current = current->next;
	}
}
