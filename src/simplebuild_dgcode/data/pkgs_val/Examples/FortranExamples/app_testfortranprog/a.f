      program add
c  Just a dummy example, squaring a number
      implicit none
      real x,y
      x = 3.0
      y = x * x
10    format ('The square of ',f4.2,' is ',f4.2)
      write (*,10) x,y
      end
