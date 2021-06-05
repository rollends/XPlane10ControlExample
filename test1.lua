function PitchController(pitchAngle, pitchAngularVelocity)
    local integrator = 0
    local Kp = 0.2
    local Kd = 0.05
    local Ki = 0.01

    while true do
        local controlAction = 0

        integrator = integrator + (2.5 - pitchAngle) * 0.05

        controlAction = Kp * (2.5 - pitchAngle)
        controlAction = controlAction + Kd * (-pitchAngularVelocity)
        controlAction = controlAction + Ki * integrator
        
        if controlAction > 1 then
            controlAction = 1
        elseif controlAction < -1 then
            controlAction = -1
        end

        pitchAngle, pitchAngularVelocity = coroutine.yield(controlAction)
    end
end
function RollController(pitchAngle, pitchAngularVelocity)
    while true do
        local controlAction = 0
        local Kp = 0.1;
        local Kd = 0.05;
        controlAction = Kp * (-pitchAngle) + Kd * (-pitchAngularVelocity);

        if controlAction > 1 then
            controlAction = 1
        elseif controlAction < -1 then
            controlAction = -1
        end

        pitchAngle, pitchAngularVelocity = coroutine.yield(controlAction)
    end
end

PitchControllerCoroutine = coroutine.create(PitchController)
RollControllerCoroutine = coroutine.create(RollController)