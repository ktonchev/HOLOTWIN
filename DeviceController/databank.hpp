#pragma once
#include <stdint.h>
#include <vector>
#include <iostream>

//Enumenator for different types of access to datapairs
enum access_types {
	NOACCESS = 0,
	GETTABLE,
	SETTABLE,
	BOTH
};

//Datapair class
class dataPair {

public:

	//Constructor
	dataPair(uint32_t id, uint32_t value, access_types access) {
		this->id = id;
		this->value = value;
		this->access = access;
	}
	
	access_types access = NOACCESS;
	uint32_t id = 1;
	uint32_t value = 0;

};


//Databank class
class dataBank {

public:
	//constructor
	dataBank(void) {
	
	}

	//Function used to extract data from dataBank
	int getDataPairById(uint32_t id, uint32_t &val) {
		int index = -1;
		//if databank is not empty, search for data pair with specific id
		if (this->dataPairs.size() > 0) {
			//searching
			for (int i = 0; i < this->dataPairs.size(); i++) {
				if (this->dataPairs[i].id == id) {
					index = i;
					break;
				}
			}
			//if there is no data pair with that id...
			if (index < 0) {
				std::cout << "No data pair with id = " << id << " found";
				return 2;
			}
			//...and if there is
			else {
				dataPair dp = this->dataPairs.at(index);
				//check access
				//if access conditions are met, copy the value into val
				if (dp.access == GETTABLE || dp.access == BOTH) {
					val = dp.value;
					return 0;
				}
				else {
					return 3;
				}
			}
		}
		else {
			std::cout << "Databank is empty";
			return 1;
		}
	}
	//Function used to change data in the databank
	int setDataPairById(uint32_t id, uint32_t value) {
		//if databank is not empty, search for data pair with specific id
		int index = -1;
		if (this->dataPairs.size() > 0) {
			for (int i = 0; i < this->dataPairs.size(); i++) {
				if (this->dataPairs[i].id == id) {
					index = i;
					break;
				}
			}
			//if there is no data pair with that id...
			if (index < 0) {
				std::cout << "No data pair with id = " << id << " found";
				return 2;
			}
			//...and if there is
			else {
				//check access
				//if access conditions are met, set the value of the data pair to a new value
				dataPair out = this->dataPairs.at(index);
				if (out.access == SETTABLE || out.access == BOTH) {
					this->dataPairs.at(index).value = value;
					return 0;
				}
				else {
					return 3;
				}
			}
		}
		else {
			dataPair out(0, 1, NOACCESS);
			std::cout << "Databank is empty";
			return 1;
		}

	}

	//Function used to add new data pairs to the data bank
	int addDataPair(dataPair pair) {
		uint32_t newPairId;
		newPairId = pair.id;
		//Check if data pair with that id already exists
		if (this->dataPairs.size() > 0) {
			for (int i = 0; i < this->dataPairs.size(); i++) {
				if (this->dataPairs[i].id == newPairId) {
					std::cout << "Data pair with id = " << newPairId << " already exists!";
					return 1;
				}
			}
		}

		this->dataPairs.push_back(pair);
		return 0;
	}

	//Delete data pair from data bank
	int deleteDataPairById(uint32_t id) {
		int index = -1;
		//if the databank is not empty, sarch for a data pair with a specific id
		if (this->dataPairs.size() > 0) {
			for (int i = 0; i < this->dataPairs.size(); i++) {
				if (this->dataPairs[i].id == id) {
					index = i;
				}
			}
			//if no data pair is found, with that id - return 0
			if (index < 0) {
				std::cout << "No data pair with id = " << id << " found";
				return 1;
			}
			//otherways delete the data pair from the dataPairs vector and return 1
			else {
				this->dataPairs.erase(this->dataPairs.begin() + index);
				return 0;
			}
		}
		else {
			std::cout << "Databank is empty";
			return 1;
		}
	}

private:
	std::vector<dataPair> dataPairs;


};
