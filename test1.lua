function MakePitchController()
    local Kp = 0.05
    local Kd = 0.02
    local Ki = 0.01
    local deltaT = 0.05

    local feedbackBlock =
        SumBlock(
            ConstantBlock(2.5), -- Desired (Reference) Pitch
            GainBlock(
                FromXPlaneBlock("sim/cockpit/gyros/the_vac_ind_deg"), -- Read Pitch Data.
                -1 -- Negative Feedback
            )
        )

    local controllerBlock =
        SaturatorBlock(
            SumBlock(
                GainBlock(
                    feedbackBlock,
                    Kp
                ),
                GainBlock(
                    IntegratorBlock(
                        feedbackBlock,
                        deltaT
                    ),
                    Ki
                ),
                GainBlock(
                    DerivativeBlock(
                        feedbackBlock,
                        deltaT
                    ),
                    Kd
                )
            ),
            -1, -- minimum
            1 -- maximum
        )

    function PitchController()
        -- Acquire Reference to Yoke Pitch Axis
        local yoke_pitch = xplane.findDataRef("sim/joystick/yoke_pitch_ratio")

        repeat
            -- Resume Control System now that the state has updated
            -- and acquire the new control action.
            local succeeded, u = coroutine.resume(controllerBlock)

            if (not succeeded) then
                error(u)
            end

            -- We have the control action. Actuate
            xplane.writeDataFloat(yoke_pitch, u)

            -- Yield
            coroutine.yield()

        until false -- Repeat Forever
    end

    return coroutine.create(PitchController)
end

function MakeRollController()
    local Kp = 0.05
    local Kd = 0.02
    local deltaT = 0.05

    local feedbackBlock =
        SumBlock(
            ConstantBlock(0), -- Desired (Reference) Roll
            GainBlock(
                FromXPlaneBlock("sim/cockpit/gyros/phi_vac_ind_deg"), -- Read Roll Data.
                -1 -- Negative Feedback
            )
        )

    local controllerBlock =
        SaturatorBlock(
            SumBlock(
                GainBlock(
                    feedbackBlock,
                    Kp
                ),
                GainBlock(
                    DerivativeBlock(
                        feedbackBlock,
                        deltaT
                    ),
                    Kd
                )
            ),
            -1, -- minimum
            1 -- maximum
        )

    function RollController()
        -- Acquire Reference to Yoke Pitch Axis
        local yoke_roll = xplane.findDataRef("sim/joystick/yoke_roll_ratio")

        repeat
            -- Resume Control System now that the state has updated
            -- and acquire the new control action.
            local succeeded, u = coroutine.resume(controllerBlock)

            if (not succeeded) then
                error(u)
            end

            -- We have the control action. Actuate.
            xplane.writeDataFloat(yoke_roll, u)

            -- Yield
            coroutine.yield()

        until false -- Repeat Forever
    end

    return coroutine.create(RollController)
end

function MakeYawController()
    local Kp = 0.05
    local Kd = 0.01
    local deltaT = 0.05

    local feedbackBlock =
        SumBlock(
            ConstantBlock(0), -- Desired (Reference)
            GainBlock(
                FromXPlaneBlock("sim/flightmodel/misc/slip"), -- Read Slip Data.
                1 -- Negative Feedback
            )
        )

    local controllerBlock =
        SaturatorBlock(
            SumBlock(
                GainBlock(
                    feedbackBlock,
                    Kp
                ),
                GainBlock(
                    DerivativeBlock(
                        feedbackBlock,
                        deltaT
                    ),
                    Kd
                )
            ),
            -1, -- minimum
            1 -- maximum
        )

    function YawController()
        -- Acquire Reference to Yoke Pitch Axis
        local yoke_yaw = xplane.findDataRef("sim/joystick/yoke_heading_ratio")

        repeat
            -- Resume Control System now that the state has updated
            -- and acquire the new control action.
            local succeeded, u = coroutine.resume(controllerBlock)

            if (not succeeded) then
                error(u)
            end

            -- We have the control action. Actuate.
            xplane.writeDataFloat(yoke_yaw, u)

            -- Yield
            coroutine.yield()

        until false -- Repeat Forever
    end

    return coroutine.create(YawController)
end

-- Construct Control Systems
PitchControllerCoroutine = MakePitchController()
RollControllerCoroutine = MakeRollController()
YawControllerCoroutine = MakeYawController()

-- Register Control Systems as part of Flight Loop
xplane.registerFlightLoop(PitchControllerCoroutine)
xplane.registerFlightLoop(RollControllerCoroutine)
xplane.registerFlightLoop(YawControllerCoroutine)