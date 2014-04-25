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
	//Enable property change listeners to call propertyChangeListener when a property changes
	setPropertyChangeListener("Modulation Type", this,
			&FMDemodulator_i::propertyChangeListener);
	setPropertyChangeListener("Bandwidth", this,
			&FMDemodulator_i::propertyBandwidthChanged);
	setPropertyChangeListener("Modulation Index", this,
			&FMDemodulator_i::propertyChangeListener);

	//options
	m_demod = NULL;

	m_delta = 0.0;							// input xdelta
	m_size = 8196;							// input size
	m_filterObjectCreated = false;			//filter created flag

	//initialize variables
	m_lastSize = 0;
	m_propertyChanged = true;
	m_bandwidthChanged = true;
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

		if(Modulation_Type == "LIQUID_FREQDEM_DELAYCONJ"){
			m_type = LIQUID_FREQDEM_DELAYCONJ;
		}else if(Modulation_Type == "LIQUID_FREQDEM_PLL"){
			m_type = LIQUID_FREQDEM_PLL;
		}else{
			std::cerr << "No Valid Modulation Type" << std::endl;
			return NOOP;
		}
		m_delta = input->SRI.xdelta;
		Sample_Rate = 1.0 / m_delta;

		createDemod();
		sizeVectors();

	    //Push new SRI data
		input->SRI.mode = 0; //outputs real data
		dataFloat_out->pushSRI(input->SRI);
		m_propertyChanged = false;
		m_bandwidthChanged = false;
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
	if(Modulation_Index == 0 || m_bandwidthChanged){
		m_modIndex = m_delta*Bandwidth;
		Modulation_Index = m_modIndex;
	}else{
		m_modIndex = Modulation_Index;
		Bandwidth = m_modIndex / m_delta;
	}

	checkProperties();

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
	m_propertyChanged=true;
}

void FMDemodulator_i::propertyBandwidthChanged(const std::string& id)
{
	//Set flag showing that properties and bandwidth changed
	m_propertyChanged=true;
	m_bandwidthChanged=true;
}

void FMDemodulator_i::checkProperties(void){
	//Check m_modIndex is valid
	if (m_modIndex<=0 || m_modIndex>1){
		m_modIndex = 0.5;
		Modulation_Index = 0.5;
		Bandwidth = Sample_Rate/2;
		std::cerr << "WARNING! -- 'Modulation Index' Must Be Greater Than Zero And Less"
				"Than Or Equal To One!" << std::endl;
		std::cerr << "-- 'Modulation Index' Set to 0.5" << std::endl;
	}

	//Check that Bandwidth is valid
	if (Bandwidth > (Sample_Rate/2) || Bandwidth <= 0) {
		Bandwidth = Sample_Rate/2;
		m_modIndex = 0.5;
		Modulation_Index = 0.5;
		std::cerr << "WARNING! -- 'Bandwidth' Must Be Equal to or Less Than 0.5*'Sample Rate'!" << std::endl;
		std::cerr << "-- 'Bandwidth' Set to 0.5*'Output_Rate'" << std::endl;
	}
}

