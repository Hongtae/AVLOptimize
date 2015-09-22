//
//  File: DKTimer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
//#include "../DKInclude.h"

#define DKGL_API

namespace DKFoundation
{
	typedef unsigned long		DKTimeTick;		// counter type default.
	typedef unsigned int		DKTimeTick32;	// 32bit counter type.
	typedef unsigned long long	DKTimeTick64;	// 64bit counter type.

	class DKGL_API DKTimer
	{
	public:
		typedef DKTimeTick64 Tick; // using 64 bit counter.
		DKTimer(void);
		~DKTimer(void);

		// reset timer, returns time elapsed since previous reset.
		double Reset(void);
		// returns elapsed timer since time since last reset.
		double Elapsed(void) const;

		static Tick SystemTick(void);          // system tick
		static Tick SystemTickFrequency(void); // tick frequency (a sec)
	private:
		Tick timeStamp;
	};
}
