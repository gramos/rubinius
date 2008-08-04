require 'sexp/exceptions'

class SimpleSexpProcessor
  def initialize
    @default_object = nil
    @ignore_return = false
    @auto_shift_type = false
  end
  
  attr_accessor :default_object, :auto_shift_type
  
  def self.process(what, *args)
    x = new(*args)
    x.process(what)
  end
  
  def ignore_return!
    @ignore_return = true
    @default_object = []
  end
  
  def require_expected=(val)
    @ignore_return = !val
  end
  
  def strict=(val)
    if !val
      @default_object = []
    else
      @default_object = nil
    end
  end
  
  def expected=(kls)
  end
  
  def process(x)
    return nil if x.nil?
    
    unless Array === x
      if respond_to? :location
        raise RuntimeError, "BUG: Invalid sexp: #{x.inspect} near #{location}"
      else        
        raise RuntimeError, "BUG: Invalid sexp: #{x.inspect}"
      end
    end

    sel = "process_#{x.first}".to_sym
    if @auto_shift_type
      name = x.shift
    else
      name = x.first
    end
    
    if @default_object
      if respond_to?(sel)
        return __send__(sel, x)
      else
        obj = @default_object.dup
        if @auto_shift_type
          obj << name
        else
          obj << x.shift
        end

        return concat_all(obj, x)
      end
    elsif respond_to?(sel)
      return __send__(sel, x)
    else
      raise RuntimeError, "Unable to find processor for type '#{name.inspect}'. Rest: #{x.inspect}"
    end
  end
  
  def concat_all(obj, x)
    x.size.times do
      e = x.shift
      if Array === e
        obj << process(e)
      else
        obj << e
      end
    end
    return obj
  end
end
