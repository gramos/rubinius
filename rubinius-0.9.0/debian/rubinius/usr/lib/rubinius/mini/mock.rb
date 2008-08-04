class MockExpectationError < StandardError; end

require 'mini/test'

class Mini::Mock
  def initialize
    @expected_calls = {}
    @actual_calls = Hash.new {|h,k| h[k] = [] }
  end

  def expect(name, retval, args=[])
    n, r, a = name, retval, args # for the closure below
    @expected_calls[name] = { :retval => retval, :args => args }
    self.class.__send__(:define_method, name) { |*a|
      raise ArgumentError unless @expected_calls[n][:args].size == a.size
      @actual_calls[n] << { :retval => r, :args => a }
      retval
    }
    self
  end

  def verify
    @expected_calls.each_key do |name|
      expected = @expected_calls[name]
      msg = "expected #{name}, #{expected.inspect}"
      raise MockExpectationError, msg unless
        @actual_calls.has_key? name and @actual_calls[name].include?(expected)
    end
    true
  end
end
