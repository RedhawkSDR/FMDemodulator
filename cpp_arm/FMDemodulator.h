/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of FMDemodulator.
 *
 * FMDemodulator is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * FMDemodulator is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef FMDEMODULATOR_IMPL_H
#define FMDEMODULATOR_IMPL_H

#include "FMDemodulator_base.h"
#include <stdio.h>
#include <liquid.h>
#include <complex>

class FMDemodulator_i;

class FMDemodulator_i : public FMDemodulator_base
{
    ENABLE_LOGGING
    public:
        FMDemodulator_i(const char *uuid, const char *label);
        ~FMDemodulator_i();
        int serviceFunction();
    private:
        //Data Vectors
		std::vector<float> m_output;

		//Demod Object Variables
		liquid_freqdem_type m_type;
		freqdem m_demod;
		bool m_filterObjectCreated;

		//SRI Data
    	double m_delta;
		unsigned int m_size;
		unsigned int m_lastSize;
		BULKIO::StreamSRI m_sriOut;

		//Member Functions
    	void sizeVectors(void);
    	void createDemod(void);

    	//Property Change Listeners
    	void modulationTypeChanged(const std::string *oldValue, const std::string *newValue);
    	void bandwidthChanged(const float *oldValue, const float *newValue);
    	void modulationIndexChanged(const float *oldValue, const float *newValue);

    	boost::mutex propertyLock_;
};

#endif
