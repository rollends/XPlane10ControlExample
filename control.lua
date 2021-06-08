function IntegratorBlock(fromBlock, dt)
    function Integrator()
        local x = 0

        repeat
            -- Wait for Input 
            local succeeded, u = coroutine.resume(fromBlock)
            
            if (not succeeded) then
                error(u)
            end

            -- Compute Integral (dt * input)
            x = x + u * dt

            -- Compute Output 
            local y = x

            -- Yield Output
            coroutine.yield(y)

        until false -- Repeat Forever
    end
    return coroutine.create(Integrator)
end

function DerivativeBlock(fromBlock, dt)
    function Derivative()
        local _, lastKnownInput = coroutine.resume(fromBlock)
        local u = lastKnownInput

        repeat
            -- Estimate the Derivative (rate of change) by taking a
            -- numerical difference and dividing by change in time.
            local estimated_derivative = (u - lastKnownInput) / dt
            
            -- Yield estimate derivative
            coroutine.yield(estimated_derivative)

            -- Store Value as "Previous" input data.
            lastKnownInput = u

            -- Resume Input Blocks to Read New Input Sample.
            succeeded, u = coroutine.resume(fromBlock)

            
            if (not succeeded) then
                error(u)
            end

        until false -- Repeat Forever
    end
    return coroutine.create(Derivative)
end

function SumBlock(u1, u2, ...)
    local inputs = {u1, u2, ...}

    function Sum()
        repeat
            local y = 0.0
            -- Resume Every Input Block to receive their outputs
            for i, inputBlock in ipairs(inputs) do
                local succeeded, ui = coroutine.resume(inputBlock)

                if (not succeeded) then
                    error(ui)
                end

                y = y + ui
            end

            -- Yield Net Sum
            coroutine.yield(y)

        until false -- Repeat forever 
    end
    return coroutine.create(Sum)
end

function GainBlock(fromBlock, K)
    function Gain()
        repeat

            local succeeded, u = coroutine.resume(fromBlock)

            if (not succeeded) then
                error(u)
            end

            local y = K * u
            coroutine.yield(y)

        until false -- Repeat forever 
    end
    return coroutine.create(Gain)
end

function SaturatorBlock(fromBlock, min, max)
    function Saturator()
        repeat

            local succeeded, u = coroutine.resume(fromBlock)
            
            if (not succeeded) then
                error(u)
            end

            local y = 0

            -- Saturate Signal between Min and Max
            if u < min then 
                y = min
            elseif u > max then
                y = max
            else
                y = u
            end

            coroutine.yield(y)

        until false -- Repeat forever 
    end
    return coroutine.create(Saturator)
end

function LowPassFilterBlock(fromBlock, K, tau, dt)
    function LowPassFilter()
        local x = 0

        repeat

            local succeeded, u = coroutine.resume(fromBlock)
            
            if (not succeeded) then
                error(u)
            end

            -- [Crude] Discrete time implementation of 
            -- G(s) = K/(\tau s + 1)
            x = x + (-1.0/tau * x + K/tau * u) * dt

            -- Produce Output
            local y = x

            coroutine.yield(y)

        until false -- Repeat forever 
    end
    return coroutine.create(Saturator)
end

function ConstantBlock(r)
    function Constant()
        repeat
            coroutine.yield(r)
        until false
    end
    return coroutine.create(Constant)
end

function FromXPlaneBlock(name)
    function Output()
        -- On first yield, make sure we acquire a reference
        -- to this signal!
        -- This is important for performance reasons!
        -- If we do not do this, we will have to look up the 
        -- signal by name (string) which will be SLOW
        local data_reference = xplane.findDataRef(name)

        repeat
            
            -- Read Data
            local data = xplane.readDataFloat(data_reference)

            -- Yield Data
            coroutine.yield(data)

        until false
    end
    return coroutine.create(Output)
end