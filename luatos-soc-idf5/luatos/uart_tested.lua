PROJECT = "uart_fixed"
VERSION = "1.0.1"

sys = require("sys")
if wdt then
    wdt.init(9000)
    sys.timerLoopStart(wdt.feed, 3000)
end

local uartid = 1 -- Using UART0

-- Initialize UART with proper parameters
uart.setup(
    uartid,
    115200,
    8,      -- Data bits
    1,      -- Stop bit
    uart.NONE,  -- Parity (explicitly set)
    uart.LSB  -- Bit order (optional)
)

-- Clear RX buffer before starting
uart.rxClear(uartid)

-- Improved receive handler
uart.on(uartid, "receive", function(id, len)
    -- Read ALL available data (not just 128 bytes)
    local data = uart.read(1, 128)
    -- if data and #data > 0 then
        -- log.info("uart", "received", #data, "bytes:", data)
        -- For hex output (uncomment if needed):
        -- log.info("uart", "hex", data:toHex())
        uart.write(uartid, data)
    -- end
end)

-- Periodic transmission test
sys.timerLoopStart(function()
    uart.write(uartid, "TEST\n") -- Added newline
    
    log.info("uart", "sent test")
end, 1000)

sys.run()