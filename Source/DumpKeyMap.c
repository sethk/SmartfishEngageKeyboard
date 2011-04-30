/*-
 * Copyright (c) 2011, Seth Kingsley
 * Copyright (c) 1999-2009 Apple Computer, Inc.	 All Rights Reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * • Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 *   disclaimer.
 * • Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided with the distribution.
 * • Neither the name Seth Kingsley nor the names of other contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <libkern/OSTypes.h>
#include <libkern/OSByteOrder.h>
#include <IOKit/hidsystem/ev_keymap.h>
#include <stdio.h>

//
// Support goop for parseKeyMapping.  These routines are
// used to walk through the keymapping string.	The string
// may be composed of bytes or shorts.	If using shorts, it
// MUST always be aligned to use short boundries.
//
typedef struct {
	unsigned const char *bp;
	unsigned const char *endPtr;
	int shorts;
} NewMappingData;

static inline unsigned int NextNum(NewMappingData *nmd)
{
	if (nmd->bp >= nmd->endPtr)
		return(0);
	if (nmd->shorts)
		return OSSwapBigToHostInt16(*((unsigned short *)nmd->bp)++);
	else
		return (*((unsigned char *)nmd->bp)++);
}

//
// Perform the actual parsing operation on a keymap.  Returns false on failure.
//

static bool
parseKeyMapping(const UInt8 *mapping, UInt32 mappingLength, NXParsedKeyMapping *parsedMapping)
{
	NewMappingData nmd;
	int i, j, k, l, n;
	unsigned int m;
	int keyMask, numMods;
	int maxSeqNum = -1;
	unsigned char *			bp; 
	
	
	/* Initialize the new map. */
	bzero( parsedMapping, sizeof (NXParsedKeyMapping) );
	parsedMapping->maxMod = -1;
	parsedMapping->numDefs = -1;
	parsedMapping->numSeqs = -1;
	
	if (!mapping || !mappingLength)
		return false;
	
	nmd.endPtr = mapping + mappingLength;
	nmd.bp = mapping;
	nmd.shorts = 1;		// First value, the size, is always a short
	
	/* Start filling it in with the new data */
	parsedMapping->mapping = (unsigned char *)mapping;
	parsedMapping->mappingLen = mappingLength;
	parsedMapping->shorts = nmd.shorts = NextNum(&nmd);
	
	/* Walk through the modifier definitions */
	numMods = NextNum(&nmd);
	for(i=0; i<numMods; i++)
	{
		/* Get bit number */
		if ((j = NextNum(&nmd)) >= NX_NUMMODIFIERS)
			return false;
		
		/* Check maxMod */
		if (j > parsedMapping->maxMod)
			parsedMapping->maxMod = j;
		
		/* record position of this def */
		parsedMapping->modDefs[j] = (unsigned char *)nmd.bp;
		
		/* Loop through each key assigned to this bit */
		for(k=0,n = NextNum(&nmd);k<n;k++)
		{
			/* Check that key code is valid */
			if ((l = NextNum(&nmd)) >= NX_NUMKEYCODES)
				return false;
			/* Make sure the key's not already assigned */
			if (parsedMapping->keyBits[l] & NX_MODMASK)
				return false;
			/* Set bit for modifier and which one */
			
			//The "if" here is to patch the keymapping file.  That file has nothing
			// for num lock, so no change is required here for num lock.
			// Also, laptop Macs have num lock handled by Buttons driver
			if ((j != NX_MODIFIERKEY_ALPHALOCK) || (/*_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK)*/true) )
			{
				parsedMapping->keyBits[l] |=NX_MODMASK | (j & NX_WHICHMODMASK);
			}
			
		}
	}
	
	//This is here because keymapping file has an entry for caps lock, but in
	//	order to trigger special code (line 646-), the entry needs to be zero
	if (/*!_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK)*/false)
		parsedMapping->modDefs[NX_MODIFIERKEY_ALPHALOCK] = 0;  
	
	//This section is here to force keymapping to include the PowerBook's secondary
	// fn key as a new modifier key.  This code can be removed once the keymapping
	// file has the fn key (ADB=0x3f) in the modifiers section.	 
	// NX_MODIFIERKEY_SECONDARYFN = 8 in ev_keymap.h
	if (/*_delegate->interfaceID() == NX_EVS_DEVICE_INTERFACE_ADB*/true)
	{
		parsedMapping->keyBits[0x3f] |=NX_MODMASK | (NX_MODIFIERKEY_SECONDARYFN & NX_WHICHMODMASK);
	}
	
	/* Walk through each key definition */
	parsedMapping->numDefs = NextNum(&nmd);
	n = parsedMapping->numDefs;
	for( i=0; i < NX_NUMKEYCODES; i++)
	{
		if (i < n)
		{
			parsedMapping->keyDefs[i] = (unsigned char *)nmd.bp;
			if ((keyMask = NextNum(&nmd)) != (nmd.shorts ? 0xFFFF: 0x00FF))
			{
				/* Set char gen bit for this guy: not a no-op */
				parsedMapping->keyBits[i] |= NX_CHARGENMASK;
				/* Check key defs to find max sequence number */
				for(j=0, k=1; j<=parsedMapping->maxMod; j++, keyMask>>=1)
				{
					if (keyMask & 0x01)
						k*= 2;
				}
				for(j=0; j<k; j++)
				{
					m = NextNum(&nmd);
					l = NextNum(&nmd);
					if (m == (unsigned)(nmd.shorts ? 0xFFFF: 0x00FF))
						if (((int)l) > maxSeqNum)
							maxSeqNum = l;	/* Update expected # of seqs */
				}
			}
			else /* unused code within active range */
				parsedMapping->keyDefs[i] = NULL;
		}
		else /* Unused code past active range */
		{
			parsedMapping->keyDefs[i] = NULL;
		}
	}
	/* Walk through sequence defs */
	parsedMapping->numSeqs = NextNum(&nmd);
	/* If the map calls more sequences than are declared, bail out */
	if (parsedMapping->numSeqs <= maxSeqNum)
		return false;
	
	/* Walk past all sequences */
	for(i = 0; i < parsedMapping->numSeqs; i++)
	{
		parsedMapping->seqDefs[i] = (unsigned char *)nmd.bp;
		/* Walk thru entries in a seq. */
		for(j=0, l=NextNum(&nmd); j<l; j++)
		{
			NextNum(&nmd);
			NextNum(&nmd);
		}
	}
	/* Install Special device keys.	 These override default values. */
	numMods = NextNum(&nmd);	/* Zero on old style keymaps */
	parsedMapping->numSpecialKeys = numMods;
	if ( numMods > NX_NUMSPECIALKEYS )
		return false;
	if ( numMods )
	{
		for ( i = 0; i < NX_NUMSPECIALKEYS; ++i )
			parsedMapping->specialKeys[i] = NX_NOSPECIALKEY;
		
		//This "if" will cover both ADB and USB keyboards.	This code does not
		//	have to be here if the keymaps include these two entries.  Keyboard
		//	drivers already have these entries, but keymapping file does not
		if (true/*_delegate->interfaceID() == NX_EVS_DEVICE_INTERFACE_ADB*/)
		{
			//ADB capslock:
			parsedMapping->specialKeys[NX_KEYTYPE_CAPS_LOCK] = 0x39;
			
			//ADB numlock for external keyboards, not PowerBook keyboards:
			parsedMapping->specialKeys[NX_KEYTYPE_NUM_LOCK] = 0x47; 
			
			//HELP key needs to be visible
			parsedMapping->keyDefs[0x72] = parsedMapping->keyDefs[0x47];
		}
		
		//Keymapping file can override caps and num lock above now:
		for ( i = 0; i < numMods; ++i )
		{
			j = NextNum(&nmd);	/* Which modifier key? */
			l = NextNum(&nmd);	/* Scancode for modifier key */
			if ( j >= NX_NUMSPECIALKEYS )
				return false;
			parsedMapping->specialKeys[j] = l;
		}
	}

	/* Install bits for Special device keys */
	for(i=0; i<NX_NUM_SCANNED_SPECIALKEYS; i++)
	{
		if ( parsedMapping->specialKeys[i] != NX_NOSPECIALKEY )
		{
			if (parsedMapping->specialKeys[i] < NX_NUMKEYCODES)
				parsedMapping->keyBits[parsedMapping->specialKeys[i]] |= (NX_CHARGENMASK | NX_SPECIALKEYMASK);
		}
	}
	
	//caps lock keys should not generate characters.
	if (true/*_delegate->doesKeyLock(NX_KEYTYPE_CAPS_LOCK)*/)
	{
		if (parsedMapping->specialKeys[NX_KEYTYPE_CAPS_LOCK] < NX_NUMKEYCODES)
			parsedMapping->keyBits[ parsedMapping->specialKeys[NX_KEYTYPE_CAPS_LOCK] ] &= ~NX_CHARGENMASK;
	}
	
	//Find scan code corresponding to PowerBook fn key (0x3f in ADB)
	//	 and then make sure it does not generate a character
	bp = parsedMapping->modDefs[NX_MODIFIERKEY_SECONDARYFN];  //7th array entry
	if (bp)
	{
		bp++;  //now points to actual ADB scan code
		parsedMapping->keyBits[ *bp ] &= ~NX_CHARGENMASK;
	}
	
	return true;
}

int
main(void)
{
	NXParsedKeyMapping parsedMapping;
#include "KeyMap.cpp"

	if (parseKeyMapping(kKeyMap, sizeof(kKeyMap), &parsedMapping))
	{
		for (int key = 0; key < NX_NUMKEYCODES; ++key)
		{
			u_char bits = parsedMapping.keyBits[key];
			fprintf(stderr, "0x%02x:%s%s%s%s\n", key,
					(bits & NX_MODMASK) ? " MODMASK" : "",
					(bits & NX_CHARGENMASK) ? " CHARGEN" : "",
					(bits & NX_SPECIALKEYMASK) ? " SPECIALKEY" : "",
					(bits & NX_KEYSTATEMASK) ? " KEYSTATE" : "");
		}
		fprintf(stderr, "success\n");
		return 0;
	}
	else
		return 1;
}