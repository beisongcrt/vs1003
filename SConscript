import os
import rtconfig
from building import *

cwd = GetCurrentDir()
src	= Glob('*.c')

if GetDepend('VS1003_EXAMPLE'):
    src += Glob('examples/*.c')
    
CPPPATH = [cwd]

group = DefineGroup('vs1003', src, depend = ['PKG_USING_VS1003'], CPPPATH = CPPPATH)

Return('group')

