#ifndef DATAREADER_HPP
#define DATAREADER_HPP

#include <thread>

#include "../Config/Types.hpp"
#include "../Config/Constants.hpp"
#include "../Utility/Ringbuffer.hpp"

/*! GrayBatReader
 *  @brief
 */

template <class DataType, class Cage>
class GrayBatReader {

private:
	typedef typename Cage::Event  Event;
    typedef typename Cage::Vertex Vertex;
	typedef typename Cage::Edge Edge;
	
	Ringbuffer<DataType> iBuffer;
	Cage& cage;
	bool done;
	std::thread receivingThread;
	
	void run() {
		assert(cage.hostedVertices.size() > 0);
		while(!done) {
			Vertex sink = cage.hostedVertices.at(0);
			std::vector<DataType> receive_buffer(1);
			sink.collect(receive_buffer);
			std::cout << "Receive" << std::endl;
			iBuffer.push(receive_buffer[0]);
		}
	}
public:
    /**
     */
    GrayBatReader(Cage& cage) :
		iBuffer(CHUNK_BUFFER_COUNT, 1),
		//cp(masterUri, selfUri, 2),
		cage(cage),
		done(false),
		receivingThread(&GrayBatReader::run, this)
	{
		std::cout << "Graybat Reader created." << std::endl;
	}
	
	
    ~GrayBatReader() {receivingThread.join();}

    void readToBuffer() {}

	Ringbuffer<DataType>* getBuffer() {
		return &iBuffer;
	}
};

#endif