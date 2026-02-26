local start = os.clock()

local n = 10000000
while n > 0 do
    n = n - 1
    local _ = 2 * 2
end

local elapsed = os.clock() - start
print(string.format("%.3fms", elapsed * 1000))