/*	  Copyright (C) 2000,2001,2002  Sony Computer Entertainment America

       	  This file is subject to the terms and conditions of the GNU Lesser
	  General Public License Version 2.1. See the file "COPYING" in the
	  main directory of this archive for more details.                             */

#ifndef ps2s_core_h
#define ps2s_core_h

#include <malloc.h>

//  #include "eetypes.h"
//  #include "eestruct.h"
#include "ps2s/types.h"
#include "ps2s/debug.h"

namespace Core {

   /********************************************
    * MemMappings
    */

   namespace MemMappings
   {
      static const tU32 Normal = 0x00000000,
#ifndef PS2_LINUX
	 Uncached = 0x20000000,
	 UncachedAccl = 0x30000000,
#else
	 Uncached = 0x00000000,
	 UncachedAccl = 0x00000000,
#endif

	 SP = 0x70000000,

	 VU0Code = 0x11000000,
	 VU0Data = 0x11004000,
	 VU1Code = 0x11008000,
	 VU1Data = 0x1100c000;
   }

   template <class ptrType>
   inline ptrType MakePtrNormal( ptrType ptr ) {
#ifndef PS2_LINUX
      return reinterpret_cast<ptrType>( (tU32)ptr & 0x0fffffff );
#else
      // since we're in a virtual address space, the upper 4 bits
      // are valid address bits, so we can't mask them off...
      // return reinterpret_cast<ptrType>( (tU32)ptr & 0x0fffffff );
      return reinterpret_cast<ptrType>( (tU32)ptr & 0xffffffff );
#endif
   }

   template <class ptrType>
   inline ptrType MakePtrUncached( ptrType ptr ) {
      return reinterpret_cast<ptrType>( (tU32)MakePtrNormal(ptr) | MemMappings::Uncached );
   }

   template <class ptrType>
   inline ptrType MakePtrUncachedAccl( ptrType ptr ) {
      return reinterpret_cast<ptrType>( (tU32)MakePtrNormal(ptr) | MemMappings::UncachedAccl);
   }

   /********************************************
    * misc
    */

   // used to override new() to allocate on qword boundaries (only for linux)

   // PLIN -- need to override delete as well...
   inline void* New16( size_t size ) {
#ifdef PS2_LINUX
      size = (size == 0) ? 1 : size;
      void *mem = memalign( 16, size );
      mErrorIf( mem == NULL, "Could not allocate memory, exiting." );
      return mem;
#else
      return ::operator new(size);
#endif
   }

   // cop0 counter

   inline void ZeroCount( void );
   inline tU32 GetCount( void );

   // fpu

   inline tU32 FToI4( float flp );

   // cop0 performance counter

   static const	tU32	COP0_NUM_PERF_COUNTERS	= 2;
   static const	tU32	COP0_NUM_PERF_EVENTS	= 17;

   void		SetupPerfCounters(tU32 evt_0,tU32 evt_1);	// set up,also zero + halt

   inline	void	HaltPerfCounters();				// stop both counters
   inline	void	ZeroStartPerfCounters();			// clear and start both counters

   inline	tU32	ReadPerfCounter0();				// read counter 0
   inline	tU32	ReadPerfCounter1();				// read counter 1

   // names of the events for people

   extern		const	char   	*ev0_Name[COP0_NUM_PERF_EVENTS];
   extern		const	char   	*ev1_Name[COP0_NUM_PERF_EVENTS];

}

/********************************************
 * Core inlines
 */

inline void
Core::ZeroCount( void ) {
   asm __volatile__ ("mtc0	$0, $9	\n"
		     "sync.p		  ");
}

inline tU32
Core::GetCount( void ) {
   tU32 ret;

   asm __volatile__ ("mfc0	%0, $9	\n"
		     "sync.p		  "
		     : "=r" (ret) );

   return ret;
}

inline tU32
Core::FToI4( float flp ) {
   tU32 fip;

   __asm__ __volatile__ ("qmtc2		%1, vf01	\n"
			 "vftoi4	vf01, vf01	\n"
			 "qmfc2		%0, vf01	\n"
			 : "=r" (fip)
			 : "r" (flp) );

   return fip;
}

// performance counter inlines

inline	void	Core::HaltPerfCounters()
{
   asm ("mtps	$0,0	\n"
	"sync.p		");
};

inline	void	Core::ZeroStartPerfCounters()
{
   asm	__volatile__("	mtps	$0,0			# halt performance counters \n"
		     "	sync.p				# \n"
		     "  mtpc	$0,0 			# set perfcounter 0 to zero \n"
		     "  mtpc	$0,1 			# set perfcounter 1 to zero \n"
		     "  sync.p				# \n"
		     "  lui	v0,0x8000	    	# master enable \n"
		     "  mtps	v0,0 			# truly - we rule \n"
		     :	// no output
		     :	// no input
		     :	"v0");
};

inline	tU32	Core::ReadPerfCounter0()
{
   tU32	ret;
   asm	__volatile__("	mfpc	%0,0	\n"
		     "	sync.p		"
		     : "=r" (ret) );
   return	ret;
}

inline	tU32	Core::ReadPerfCounter1()
{
   tU32	ret;
   asm	__volatile__("	mfpc	%0,1	\n"
		     "	sync.p		"
		     : "=r" (ret) );
   return	ret;
}

#endif // ps2s_core_h
