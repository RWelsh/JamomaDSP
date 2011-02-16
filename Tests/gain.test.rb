#!/usr/bin/ruby

require 'Jamoma'

o = TTObject.new "gain"
o.send "test"

err, cpu = o.send "getProcessingBenchmark", 1

puts
puts "time spent calculating audio process method: #{cpu} µs"
puts
