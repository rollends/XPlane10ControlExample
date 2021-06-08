require("control")

xplane = {
    findDataRef =
        function ()
            return {}
        end,
    readDataFloat =
        function ()
            return 0.0
        end
}

require("test1")

coroutine.resume(PitchControllerCoroutine)