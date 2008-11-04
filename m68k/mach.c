/*
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <PalmOS.h>
#include "mem.h"

struct Machine {
	UInt32 palmos_id;
	UInt32 linux_id;
	char *name;
};

/**
 * Register ARM Linux machine numbers at:
 * http://www.arm.linux.org.uk/developer/machines/
 */
struct Machine mach_table[] = {
	{ '????',    0, "Unknown device" },
	{ 'TunX',  835, "Palm LifeDrive" },
	{ 'H102',  909, "Palm Treo 650" },
	{ 'D053', 1230, "Palm Treo 680" },
	{ 'D052', 1421, "Palm Treo 700p" },
	{ 'D061', 1944, "Palm Centro (GSM)" },
	{ 'D062', 1944, "Palm Centro (CDMA)" },
	{ 'MT64',  918, "Palm Tungsten C" },
	{ 'Cct1',  817, "Palm Tungsten E" },
	{ 'Zir4',  844, "Palm Tungsten E2" },
	{ 'Frg1',  839, "Palm Tungsten T" },
	{ 'Frg2',  834, "Palm Tungsten T2" },
	{ 'Arz1',  829, "Palm Tungsten T3" },
	{ 'TnT5',  917, "Palm Tungsten T5" },
	{ 'D050',  885, "Palm TX" },
	{ 'Zi22', 1090, "Palm Zire 31" },
	{ 'Zpth',  993, "Palm Zire 71" },
	{ 'Zi72',  904, "Palm Zire 72" },

	{ 'Rdog',  779, "Tapwave Zodiac" },  
	
	{ NULL },
};

static UInt16 mach = 0;

static void check_mach()
{
	UInt16 i;
	UInt32 palmos_id;
	
	if (mach) return;
	
	FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &palmos_id);

	for (i=1; mach_table[i].palmos_id; i++) {
		if (mach_table[i].palmos_id == palmos_id) {
			mach = i;
			return;
		}
	}
	mach = 0; /* unknown */
}

UInt32 get_linux_mach_id()
{
	check_mach();
	return mach_table[mach].linux_id;
}

char *get_mach_name()
{
	check_mach();
	return mach_table[mach].name;
}

/**
 * Offset to subtract from translation table entries.
 * On treo's translation table is 1:1 mapped so this is zero.
 * On other devices the translation table stuff starts at 0x0 so we
 * subtract ram_base.
 */
UInt32 get_tt_offset()
{
	check_mach();
	switch (mach_table[mach].linux_id) {
	case 909:	/* Treo 650 */
	case 1230:	/* Treo 680 */
	case 1421:	/* Treo 700p */
	case 1944:	/* Centro */
		return 0;
	default:
		return get_ram_base();
	}
}
