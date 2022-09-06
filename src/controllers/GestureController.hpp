/*
 * Filename: GestureController.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class implementing the numerical (keyboard) controller.
 */

#if !defined(GestureController_hpp)
#define GestureController_hpp

#include <iostream>
#include "BaseController.hpp"
#include "../gesture/gestureModel.hpp"
#include "../gloveInterface/getCurrentData.hpp"

// NumericalController derived from BaseController with callback that accepts two ints that uses the serial ports and gesture models to determine the desired callback values from the hand gesture
class GestureController : public BaseController<int, int> {
public:
	// NumericalController derived from BaseController with callback that accepts two ints that uses the serial ports and gesture models to determine the desired callback values from the hand gesture
	GestureController(serialib& lserial, df_t& lmodel, serialib& rserial, df_t& rmodel, std::function<void(int, int)> callback) : lserial(lserial), lmodel(lmodel), rserial(rserial), rmodel(rmodel), BaseController<int, int>(callback) {}

	// The run function requires an SFML event to be passed, but doesn't use it.
	void run(sf::Event event) {
		// Read the raw data from the gloves
		auto ldata = getCurrentData(lserial);
		auto rdata = getCurrentData(rserial);

		// Convert from vector to sample type and predict the gesture using the gesture model
		auto lpred = lmodel(convertToSampleType(std::vector<float>(ldata.begin(), ldata.end())));
		auto rpred = rmodel(convertToSampleType(std::vector<float>(rdata.begin(), rdata.end())));

		// Use the predictions to call the callback function.
		callback((int)lpred, (int)rpred);
	}

private:
	serialib lserial, rserial;
	df_t lmodel, rmodel;
};

#endif // GestureController_hpp