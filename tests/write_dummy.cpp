// Data model
#include "EventInfoData.h"
#include "EventInfoCollection.h"
#include "ExampleMCData.h"
#include "ExampleMCCollection.h"


// STL
#include <iostream>
#include <vector>
#include <map>

// podio specific includes
#include "podio/EventStore.h"
#include "podio/CollectionBase.h"
#include "podio/HDF5Writer.h"

// HDF5 specific includes
#include "H5Cpp.h"
#include "H5_EventInfoData.h"
#include "H5_ExampleMCData.h"


using namespace H5;	
const H5std_string FILE_NAME("dummy.h5");
const H5std_string FILE_NAME_1("dummy_HDF5Writer.h5");

const H5std_string DATASET_NAME_1("EventInfo_data_1");
const H5std_string DATASET_NAME_2("EventInfo_data_2");
const H5std_string DATASET_NAME_3("ExampleMC_data_3");

const int RANK = 1;

int main()
{

	try{
		// create file
		H5File file(FILE_NAME, H5F_ACC_TRUNC);
		
		// Initialize some data that we would like to serialize
		std::cout<<"Start Processing..."<<std::endl;

		auto store = podio::EventStore();
		auto& info_1 = store.create<EventInfoCollection>("info_1");
		auto& info_2 = store.create<EventInfoCollection>("info_2");
		auto& mcps = store.create<ExampleMCCollection>("mcparticles");

		// COLLECTION 1
		auto item_1 = EventInfo();
		item_1.Number(20); 
		
		auto item_2 = EventInfo();
		item_2.Number(21);

		info_1.push_back(item_1);
		info_1.push_back(item_2);
	
		// COLLECTION 2
		auto item_3 = EventInfo();
		item_3.Number(22);
		auto item_4 = EventInfo();
		item_4.Number(23);
		auto item_5 = EventInfo();
		item_5.Number(24);
		auto item_6 = EventInfo();
		item_6.Number(25);
		
		info_2.push_back(item_3);
		info_2.push_back(item_4);
		info_2.push_back(item_5);
		info_2.push_back(item_6);


		// COLLECTION 3
		auto mcp0 = ExampleMC();
		auto mcp1 = ExampleMC();
		auto mcp2 = ExampleMC();
		auto mcp3 = ExampleMC();

		mcps.push_back( mcp0 ) ;
		mcps.push_back( mcp1 ) ;
		mcps.push_back( mcp2 ) ;
		mcps.push_back( mcp3 ) ;

		// --- add some daughter relations
		auto p = ExampleMC();
		auto d = ExampleMC();

		p = mcps[0] ;
		p.adddaughters( mcps[1] ) ;
		p.adddaughters( mcps[2] ) ;
		
		p = mcps[1] ;
		p.adddaughters( mcps[2] ) ;
		p.adddaughters( mcps[3] ) ;
		
		//--- now fix the parent relations
		for( unsigned j=0,N=mcps.size();j<N;++j)
		{
			p = mcps[j] ;
			for(auto it = p.daughters_begin(), end = p.daughters_end() ; it!=end ; ++it )
			{
				int dIndex = it->getObjectID().index ;
				d = mcps[ dIndex ] ;
				d.addparents( p ) ;
			}
		}
		

		// In the future we want ...
		// Currently the part below does not do anything yet apart from printing the collection. 
		auto writer = podio::HDF5Writer(FILE_NAME_1, &store);
		writer.registerForWrite<EventInfoCollection>("info_1");
		//writer.registerForWrite<EventInfoCollection>("info_2");
		//writer.registerForWrite<ExampleMCCollection>("mcparticles");
		writer.writeEvent(); 


		// CompTypes
		H5_EventInfoData h5eid;
		H5_ExampleMCData h5emcd; 
	
		// DATASET 1
		const hsize_t SIZE_1 = info_1.size(); 
		hsize_t dim_1[] = {SIZE_1};
		DataSpace space_1(RANK, dim_1);
		DataSet dataset1 = file.createDataSet(DATASET_NAME_1, h5eid.h5dt(), space_1);
		
		
		// DATASET 2
		const hsize_t SIZE_2 = info_2.size(); 
		hsize_t dim_2[] = {SIZE_2};
		DataSpace space_2(RANK, dim_2);
		DataSet dataset2 = file.createDataSet(DATASET_NAME_2, h5eid.h5dt(), space_2);
	

		// DATASET 3
		const hsize_t SIZE_3 = mcps.size(); 
		hsize_t dim_3[] = {SIZE_3};
		DataSpace space_3(RANK, dim_3);
		DataSet dataset3 = file.createDataSet(DATASET_NAME_3, h5emcd.h5dt(), space_3);
		
		
		// Fill 
		info_1->prepareForWrite();
		void* buffer_1 = info_1->_getBuffer();
		EventInfoData** data_1 = reinterpret_cast<EventInfoData**>(buffer_1);
	
		info_2->prepareForWrite();
		void* buffer_2 = info_2->_getBuffer();
		EventInfoData** data_2 = reinterpret_cast<EventInfoData**>(buffer_2);

		mcps->prepareForWrite();
		void* buffer_3 = mcps->_getBuffer();
		ExampleMCData** data_3 = reinterpret_cast<ExampleMCData**>(buffer_3);
		

		std::cout<<"Writing data..."<<std::endl;
		
		std::cout<<"Dataset I"<<std::endl;
		
		for(int i=0; i<2; i++)	
			std::cout<<info_1[i]<<std::endl; 
			
		std::cout<<"Dataset II"<<std::endl;

		for(int i=0; i<4; i++)	
			std::cout<<info_2[i]<<std::endl;
			
		std::cout<<"Dataset III"<<std::endl;
		
    		for( auto p : mcps )
    		{
			std::cout << " particle " << p.getObjectID().index << " has daughters: " ;
	
			for(auto it = p.daughters_begin(), end = p.daughters_end() ; it!=end ; ++it )
			{
				std::cout << " " << it->getObjectID().index ;
			}
			std::cout << "  and parents: " ;
	
			for(auto it = p.parents_begin(), end = p.parents_end() ; it!=end ; ++it )
			{
				std::cout << " " << it->getObjectID().index ;
			}
			std::cout << std::endl ;
		}
	
		// Write data to file
		
		dataset1.write(*data_1, h5eid.h5dt());	
		dataset2.write(*data_2, h5eid.h5dt()); 
		dataset2.write(buffer_3, h5emcd.h5dt()); 
		
		store.clearCollections();

	} // end of try block
	
	// catch failure caused by the H5File operations
	catch( FileIException error )
	{
		error.printErrorStack();
		return -1;
	}
	// catch failure caused by the DataSet operations
	catch( DataSetIException error )
	{
		error.printErrorStack();
		return -1;
	}
	// catch failure caused by the DataSpace operations
	catch( DataSpaceIException error )
	{
		error.printErrorStack();
		return -1;
	}
	catch(...)
	{
		std::cout<<"Something terrible happened!"<<std::endl;
		return -1;
	}
	return 0;
}
