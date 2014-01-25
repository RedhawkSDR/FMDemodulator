/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.

 * This file is part of FMDemodulator.

 * FMDemodulator is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.

 * FMDemodulator is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

 	Source: FMDemodulator.spd.xml
 	Generated on: Thu Aug 15 13:42:14 EDT 2013
 	REDHAWK IDE
 	Version: 1.8.4
 	Build id: R201305151907

**************************************************************************/

#include "FMDemodulator.h"

FMDemodulator_i::FMDemodulator_i(const char *uuid, const char *label) :
    FMDemodulator_base(uuid, label)
{
	//options
	m_modIndex = 0.125; 					// modulation index (bandwidth)
	m_bandwidth = 64000;
	m_fc = 0.0f; 							// FM carrier
	m_type = LIQUID_FREQDEM_DELAYCONJ;		// Demod type
	m_demod = 0;

	m_delta = 0.000003906;					// input xdelta
	m_size = 8196;							// input size
	m_filterObjectCreated = false;			//filter created flag

	//initialize variables
	m_lastSize = 0;
	m_propertyChanged = 1;
}

FMDemodulator_i::~FMDemodulator_i()
{
}

int FMDemodulator_i::serviceFunction()
{
	//Input Data
	bulkio::InFloatPort::dataTransfer *input = dataFloat_in->getPacket(-1);
	std::vector< std::complex<float> >* preDemod = (std::vector<std::complex<float> >*) &(input->dataBuffer);

	if (not input) {
		return NOOP;
	}

	m_size = preDemod->size();

	//Re-make demod object based on new properties
	if (m_propertyChanged || input->sriChanged) {
		createDemod();
		sizeVectors();
		m_delta = input->SRI.xdelta;

	    //Push new SRI data
		input->SRI.mode = 0; //outputs real data
		dataFloat_out->pushSRI(input->SRI);
		m_propertyChanged = 0;
	}
	//Resize vectors if input size changes
	if (m_lastSize != m_size) {sizeVectors();}

	//Convert incoming interleaved complex to C++ complex and demod data
	for (unsigned int i=0; i<m_size; i++) {
        freqdem_demodulate(m_demod, *((liquid_float_complex*)&preDemod->at(i)), &m_output[i]);
    }

    //Push output data
	dataFloat_out->pushPacket(m_output, input->T, input->EOS, input->streamID);

	delete input; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
    return NORMAL;
}

void FMDemodulator_i::createDemod(void)
{
	//Destroy Demod object if one already exists
	if (m_filterObjectCreated) {freqdem_destroy(m_demod);}

	//Set m_modIndex and create m_demod object
	m_modIndex = m_delta*m_bandwidth;
	m_demod = freqdem_create(m_modIndex,m_type);
	m_filterObjectCreated = true;
}

void FMDemodulator_i::sizeVectors(void)
{
	//Resize vectors;
	m_output.resize(m_size);
	m_lastSize = m_size;
}
void FMDemodulator_i::propertyChangeListener(const std::string& id)
{
	//Set flag showing that properties changed
	m_propertyChanged=1;
}

