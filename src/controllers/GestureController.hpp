#if !defined(GestureController_hpp)
#define GestureController_hpp

#include <iostream>
#include "BaseController.hpp"
#include "../gesture/gestureModel.hpp"
#include "../gloveInterface/getCurrentData.hpp"

class GestureController : public BaseController<int, int> {
public:
	GestureController(serialib& lserial, df_t& lmodel, serialib& rserial, df_t& rmodel, std::function<void(int, int)> callback) : lserial(lserial), lmodel(lmodel), rserial(rserial), rmodel(rmodel) BaseController<int, int>(callback) {}

	void run(sf::Event event) {
		auto ldata = getCurrentData(lserial);
		auto rdata = getCurrentData(rserial);
		auto lpred = lmodel(convertToSampleType(ldata));
		auto rpred = rmodel(convertToSampleType(rdata));
		callback((int)lpred, (int)rpred);
	}

private:
	serialib lserial, rserial;
	df_t lmodel, rmodel;
};

#endif // GestureController_hpp