#!/usr/bin/env ruby

base = File.expand_path(File.dirname(__FILE__))
top = File.expand_path(File.join(base, ".."))

$LOAD_PATH.unshift(File.join(top, "src"))
$LOAD_PATH.unshift(File.join(top, "src", "lib"))
$LOAD_PATH.unshift(base)

require 'test/unit'

exit Test::Unit::AutoRunner.run(true, base)
