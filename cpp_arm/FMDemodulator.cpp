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
	addPropertyChangeListener("Modulation Type", this, &FMDemodulator_i::modulationTypeChanged);
	addPropertyChangeListener("Bandwidth", this, &FMDemodulator_i::bandwidthChanged);
	addPropertyChangeListener("Modulation Index", this, &FMDemodulator_i::modulationIndexChanged);

	//options
	m_demod = NULL;
	m_type = LIQUID_FREQDEM_DELAYCONJ;
	m_delta = 0.0;							// input xdelta
	m_size = 0;								// input packet size
	m_lastSize = 0;							// previous packet size
	m_filterObjectCreated = false;			// filter created flag

	//Create output sri object
	m_sriOut = bulkio::sri::create("FMDemodulator_OUT");
}

FMDemodulator_i::~FMDemodulator_i()
{
}

/************************* Property Change Listeners *******************************************/
void FMDemodulator_i::modulationTypeChanged(const std::string *oldValue, const std::string *newValue)
{
	boost::mutex::scoped_lock lock(propertyLock_);

	//Set Modulation Type
	if(*newValue == "LIQUID_FREQDEM_DELAYCONJ"){
		m_type = LIQUID_FREQDEM_DELAYCONJ;
	}else if(*newValue == "LIQUID_FREQDEM_PLL"){
		m_type = LIQUID_FREQDEM_PLL;
	}else{
		std::cerr << "No Valid 'Modulation Type'." << std::endl;
		std::cerr << "-- Setting 'Modulation Type' as 'LIQUID_FREQDEM_DELAYCONJ'." << std::endl;
		m_type = LIQUID_FREQDEM_DELAYCONJ;
	}

	//Create Demod if SRI has been received
	if (m_delta != 0){
		createDemod();
	}

}
void FMDemodulator_i::bandwidthChanged(const float *oldValue, const float *newValue)
{
	boost::mutex::scoped_lock lock(propertyLock_);

	//Check for valid Bandwidth if SRI has been received
	if (m_delta !=0){
		if (*newValue > (Sample_Rate/2) || *newValue <= 0) {
			Bandwidth = *oldValue;
			std::cerr << "WARNING! -- 'Bandwidth' Must Be Equal to or Less Than 0.5*'Sample Rate' and greater than 0!" << std::endl;
			std::cerr << "-- Keeping 'Bandwidth' as " << *oldValue << std::endl;
		}else{
			Bandwidth = *newValue;
		}

		//Set Modulation Index and create demod object
		Modulation_Index = m_delta * Bandwidth;
		createDemod();
	}
}

void FMDemodulator_i::modulationIndexChanged(const float *oldValue, const float *newValue)
{
	boost::mutex::scoped_lock lock(propertyLock_);

	//Check for valid Modulation Index if SRI has been received
	if (m_delta != 0){
		if (*newValue <= 0 || *newValue > 1){
			Modulation_Index = *oldValue;
			Bandwidth = Sample_Rate/2;
			std::cerr << "WARNING! -- 'Modulation Index' Must Be Greater Than Zero And Less Than Or Equal To One!" << std::endl;
			std::cerr << "-- Keeping 'Modulation Index' as " << *oldValue << std::endl;
		}else{
			Modulation_Index = *newValue;
		}

		//Set Bandwidth and create demod object
		Bandwidth = Modulation_Index / m_delta;
		createDemod();
	}
}

/***********************************************************************************************
************************************************************************************************/
int FMDemodulator_i::serviceFunction()
{
	//Input Data
	bulkio::InFloatPort::dataTransfer *input = dataFloat_in->getPacket(-1);
	std::vector< std::complex<float> >* preDemod = (std::vector<std::complex<float> >*) &(input->dataBuffer);

	if (not input) {
		return NOOP;
	}

	m_size = preDemod->size();

	{
		boost::mutex::scoped_lock lock(propertyLock_);

		//Re-make demod object based on new properties
		if (input->sriChanged) {

			m_delta = input->SRI.xdelta;
			Sample_Rate = 1.0 / m_delta;
			Modulation_Index = m_delta * Bandwidth;

			createDemod();
			sizeVectors();

			//Push new SRI data
			m_sriOut = input->SRI;
			m_sriOut.mode = 0;
			dataFloat_out->pushSRI(m_sriOut);
		}
		//Resize vectors if input size changes
		if (m_lastSize != m_size) {
			sizeVectors();
		}

		//Demod data
		for (unsigned int i = 0; i < m_size; i++) {
			freqdem_demodulate(m_demod, *((liquid_float_complex*) &preDemod->at(i)), &m_output[i]);
		}
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

	m_demod = freqdem_create(Modulation_Index,m_type);
	m_filterObjectCreated = true;
}

void FMDemodulator_i::sizeVectors(void)
{
	//Resize vectors;
	m_output.resize(m_size);
	m_lastSize = m_size;
}

