#include "ostask_oled_ui.h"

#include "vehicle_controller.h"
#include "port_ui.h"

namespace ostask_oled_ui {

    void taskProcedure(void *argument) {
        auto* controller = static_cast<vehicle_controller *>(argument);

        // TODO: initialize OLED & KEYS (GPIO)
        PortKeys::init();

        while (true) {
            // TODO
//            cart_point target_point = {-0.3f, 1.5f};
//            controller->set_target_point(target_point);

            osDelay(20);
        }
    }

} // namespace ostask_oled_ui