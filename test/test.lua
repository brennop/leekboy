local json = require "test.json"
local ffi = require "ffi"

local bor, band, bxor, lshift, rshift = bit.bor, bit.band, bit.bxor, bit.lshift, bit.rshift

local lib = ffi.load("lib/libleekboy.so")

local function load_header(name)
  local file = io.open("include/" .. name .. ".h", "r")
  local data = ""
  for line in file:lines() do
    if not line:match("^#") then
      data = data .. line .. "\n"
    end
  end
  return data
end

local function def_header(name)
  local data = load_header(name)
  ffi.cdef(data)
end

local function load_cpu()
  local cpu = ffi.new("CPU")
  local rom = ffi.new("uint8_t[?]", 0x10000)

  lib.cpu_init(cpu, rom)

  return cpu
end

local function load_test()
  local testsuite = arg[1]
  local file = io.open(testsuite, "r")
  local data = file:read "*all"
  file:close()
  return json.decode(data)
end

local function run_test(test, cpu)
  for _, v in ipairs(test.initial.ram) do
    address, value = unpack(v)
    address, value = tonumber(address), tonumber(value)
    lib.cpu_memory_set(cpu, address, value)
  end

  -- set registers
  cpu.a = tonumber(test.initial.cpu.a)
  cpu.f = tonumber(test.initial.cpu.f)
  cpu.b = tonumber(test.initial.cpu.b)
  cpu.c = tonumber(test.initial.cpu.c)
  cpu.d = tonumber(test.initial.cpu.d)
  cpu.e = tonumber(test.initial.cpu.e)
  cpu.h = tonumber(test.initial.cpu.h)
  cpu.l = tonumber(test.initial.cpu.l)
  cpu.sp = tonumber(test.initial.cpu.sp)
  cpu.pc = tonumber(test.initial.cpu.pc)

  lib.cpu_step(cpu)

  local function assert_register(name)
    local expected = tonumber(test.final.cpu[name])
    local actual = cpu[name]

    local message = [[

    TEST FAILED
      test: %s
      register: %s
      expected: 0x%04x
      actual: 0x%04x
    ]]

    assert(actual == expected, message:format(test.name, name, expected, actual))
  end

  local function assert_flag(offset, name)
    local flags = tonumber(test.final.cpu.f)
    local expected = band(flags, lshift(1, offset))
    local actual = band(cpu.f, lshift(1, offset))

    local message = [[

    TEST FAILED
      test: %s
      flag: %s
      expected: 0x%04x
      actual: 0x%04x
    ]]

    assert(actual == expected, message:format(test.name, name, expected, actual))
  end

  -- check registers
  assert_register("a")
  assert_register("b")
  assert_register("c")
  assert_register("d")
  assert_register("e")
  assert_register("h")
  assert_register("l")
  assert_register("sp")
  assert_register("pc")

  -- check flags
  assert_flag(7, "zero")
  assert_flag(6, "sub")
  assert_flag(5, "half")
  assert_flag(4, "carry")

  -- check memory
  for _, v in ipairs(test.final.ram) do
    address, value = unpack(v)
    address, value = tonumber(address), tonumber(value)
    assert(lib.cpu_memory_get(cpu, address) == value)
  end
end

local function run_tests(tests, cpu)
  local errors = 0
  for i = 1, 10000 do
    local test = tests[i]

    local ok, err = pcall(run_test, test, cpu)

    if not ok then
      print(err)
      errors = errors + 1
    end
  end

  print(string.format("Passing: %d/%d", 10000 - errors, 10000))
end

local function main()
  def_header("memory")
  def_header("cpu")

  local tests = load_test()
  local cpu = load_cpu()
  run_tests(tests, cpu)
end

main()
