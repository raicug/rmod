#include <SDK/Interface.h>
#include "../../hooks.h"
#include <globals/settings.h>

int __fastcall raicu::hooks::handles::send_datagram(c_net_channel* net_channel, void* datagram) {
	return originals::send_datagram(net_channel, datagram);
}