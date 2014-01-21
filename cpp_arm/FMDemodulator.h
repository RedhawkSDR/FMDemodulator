/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK.
 *
 * REDHAWK is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef FMDEMODULATOR_IMPL_H
#define FMDEMODULATOR_IMPL_H

#include "FMDemodulator_base.h"
#include <stdio.h>
#include <liquid/liquid.h>
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

		liquid_freqdem_type m_type;
		freqdem m_demod;

    	float m_modIndex;
    	float m_fc;
    	float m_bandwidth;

    	double m_delta;
		unsigned int m_size;
		unsigned int m_lastSize;
		bool m_filterObjectCreated;

		int m_propertyChanged;

    	void propertyChangeListener(const std::string&);
    	void sizeVectors(void);
    	void createDemod(void);
};

#endif
