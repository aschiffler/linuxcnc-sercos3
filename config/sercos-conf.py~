import hal
import glib
import time
import os


class HandlerClass:
    '''
    class with gladevcp callback handlers
    '''

    def on_phase_nrt_toggled(self,widget,data=None):
	os.system("halcmd call sercos-conf n")

    def on_phase0_toggled(self,widget,data=None):
	os.system("halcmd call sercos-conf 0")

    def on_phase4_toggled(self,widget,data=None):
	os.system("halcmd call sercos-conf f")

    def on_phase_err_toggled(self,widget,data=None):
	os.system("halcmd call sercos-conf e")


    def __init__(self, halcomp,builder,useropts,compname):
        '''
        Handler classes are instantiated in the following state:
        - the widget tree is created, but not yet realized (no toplevel window.show() executed yet)
        - the halcomp HAL component is set up and the widhget tree's HAL pins have already been added to it
        - it is safe to add more hal pins because halcomp.ready() has not yet been called at this point.

        after all handlers are instantiated in command line and get_handlers() order, callbacks will be
        connected with connect_signals()/signal_autoconnect()

        The builder may be either of libglade or GtkBuilder type depending on the glade file format.
        '''

        self.halcomp = halcomp
        self.builder = builder
        self.nhits = 0




def get_handlers(halcomp,builder,useropts,compname):
    '''
    this function is called by gladevcp at import time (when this module is passed with '-u <modname>.py')

    return a list of object instances whose methods should be connected as callback handlers
    any method whose name does not begin with an underscore ('_') is a  callback candidate

    the 'get_handlers' name is reserved - gladevcp expects it, so do not change
    '''
    return [HandlerClass(halcomp,builder,useropts,compname)]
