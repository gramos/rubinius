#
#   output-method.rb - optput methods used by irb 
#       $Release Version: 0.9.5$
#       $Revision: 11708 $
#       $Date: 2007-02-12 15:01:19 -0800 (Mon, 12 Feb 2007) $
#       by Keiju ISHITSUKA(keiju@ruby-lang.org)
#
# --
#
#   
#

require "e2mmap"

module IRB
  # OutputMethod
  #   StdioOutputMethod

  class OutputMethod
    @RCS_ID='-$Id: output-method.rb 11708 2007-02-12 23:01:19Z shyouhei $-'

    def print(*opts)
      IRB.fail NotImplementError, "print"
    end

    def printn(*opts)
      print opts.join(" "), "\n"
    end

    # extend printf
    def printf(format, *opts)
      if /(%*)%I/ =~ format
        format, opts = parse_printf_format(format, opts)
      end
      print sprintf(format, *opts)
    end

    # %
    # <�ե饰>  [#0- +]
    # <�Ǿ��ե��������> (\*|\*[1-9][0-9]*\$|[1-9][0-9]*)
    # <����>.(\*|\*[1-9][0-9]*\$|[1-9][0-9]*|)?
    # #<Ĺ������ʸ��>(hh|h|l|ll|L|q|j|z|t)
    # <�Ѵ�����ʸ��>[diouxXeEfgGcsb%] 
    def parse_printf_format(format, opts)
      return format, opts if $1.size % 2 == 1
    end

    def puts(*objs)
      for obj in objs
        print(*obj)
        print "\n"
      end
    end

    def pp(*objs)
      puts(*objs.collect{|obj| obj.inspect})
    end

    def ppx(prefix, *objs)
      puts(*objs.collect{|obj| prefix+obj.inspect})
    end

  end

  class StdioOutputMethod<OutputMethod
    def print(*opts)
      STDOUT.print(*opts)
    end
  end
end
