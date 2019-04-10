#!/usr/bin/python


from gi.repository import Gtk, Gio, GLib
import sys, threading, time
from numpy import sin, cos, pi, linspace
from matplotlib.figure import Figure
import matplotlib.cm as cm
import sqlite3
from array import *

from matplotlib.backends.backend_gtk3cairo import FigureCanvasGTK3Cairo as FigureCanvas
from matplotlib.dates import datetime as dt
from matplotlib.dates import DateFormatter
from matplotlib import gridspec


ManSettings = Gio.Settings.new("com.lar.gui.ultimate.signal")

from os.path import expanduser


SIGNAL_RUN = True

class ReadStdin(threading.Thread):
	def __init__ (self,signal):
		threading.Thread.__init__(self)
		self.signal = signal
		self.buff = ''
	def run(self):
		while SIGNAL_RUN:
			#print "read stdin thread"
			self.buff += sys.stdin.read(1)
			if self.buff.endswith('\n'):
				print self.buff[:-1]
				self.buff = ''
			#time.sleep(5)


class Signal:
	'''Creates integration signal plot.'''
	X_Data               = []
	Y_Data               = []
	LifeZero             = 0.0
	IntegrationStart     = 0.0
	LifeZero             = 0.0
	
	def __init__(self):
		n = 1000        
		#print X
		#dataY = c.execute("SELECT data_value FROM MktSensorData WHERE data_creator = 1 and data_trigger = 1")
		#X =[]
		#Y =[]
		#
		#	X.append(row[0])
		#	Y.append(row[1])
		
		#print X
		#xsin = linspace(-pi, pi, n, endpoint=True)
		#xcos = linspace(-pi, pi, n, endpoint=True)
		#ysin = sin(xsin)
		#ycos = cos(xcos)
		#sinwave = ax.plot(dataX, dataY, color='blue', label='Signal')
		#formatter = DateFormatter('%S')
		self.fig = Figure(figsize=(10,10), dpi=80, facecolor='white')
		
		self.ax = self.fig.add_subplot(111)
		
		#ax.xaxis.set_major_formatter(formatter)

		#ax.set_xlim(-pi,pi)
		#ax.set_ylim(-1.2,1.2)
		#ax.fill_between(xsin, 0, ysin, (ysin - 1) > -1, color='blue', alpha=.3)
		#ax.fill_between(xsin, 0, ysin, (ysin - 1) < -1, color='red',  alpha=.3)
		#ax.fill_between(xcos, 0, ycos, (ycos - 1) > -1, color='blue', alpha=.3)
		#ax.fill_between(xcos, 0, ycos, (ycos - 1) < -1, color='red',  alpha=.3)
		#print "test 1!"
		#self.ax.legend(loc='upper left')
		#print "test 2!"
		#ax = fig.gca()
		#ax.spines['right'].set_color('none')
		#ax.spines['top'].set_color('none')
		#ax.xaxis.set_ticks_position('bottom')
		#ax.spines['bottom'].set_position(('data',0))
		#ax.yaxis.set_ticks_position('left')
		#ax.spines['left'].set_position(('data',0))
		#
		self.canvas = FigureCanvas(self.fig)
		#GLib.timeout_add_seconds(1, self.fast_update)
		#self.read = ReadStdin(self)
		#self.read.start()
	
	def clean_data(self):
		self.X_Data.clean()
		

	def auto_update(self):
		
		Cursor = self.Conn.cursor()
		print "SELECT ref_id,data_changed,data_value,data_type FROM "+self.data_base+" WHERE ref_id > "+str(self.last_id)+" and data_creator = "+str(self.data_creator)+" and data_trigger = "+str(self.data_trigger)+" LIMIT 6"
		data = Cursor.execute("SELECT ref_id,data_changed,data_value,data_type FROM "+self.data_base+" WHERE ref_id > "+str(self.last_id)+" and data_creator = "+str(self.data_creator)+" and data_trigger = "+str(self.data_trigger)+" LIMIT 6")
		life_zero     = 0.0
		starti        = 0.0
		stopi         = 0.0
		is_zero       = False
		r = data.fetchone()
		if r != None :
			for row in data:
				self.last_id=row[0]
				self.X.append(row[1]-self.start_time)
				self.Y.append(row[2])
				if row[3] == self.ULTIMATE_SIGNAL_JUSTIFICATED:
					life_zero = r[2]
					is_zero   = True
				if row[3] == self.ULTIMATE_SIGNAL_INTEGRATION_START:
					starti = row[1]-self.start_time
				if row[3] == self.ULTIMATE_SIGNAL_INTEGRATION_STOP:
					stopi  = row[1]-self.start_time
			
			self.ax.plot(self.X, self.Y, color='blue', label='Signal', linewidth=2)
			if is_zero == True:
				l = self.ax.axhline(y=life_zero,linewidth=1, color='r')
			if starti > 0.0:
				lsi = self.ax.axvline(x=starti,linewidth=1, color='g')
			if stopi > 0.0:
				lsti = self.ax.axvline(x=stopi,linewidth=2, color='r')
			self.canvas.draw()
		return True

			
	def update(self):

		print "Update signal plot"
		self.last_id       = 0
		self.start_time    = 0.0
		self.data_base     = ManSettings.get_value(self.DATABASE).get_string()
		self.data_creator  = ManSettings.get_value(self.REFERENCE).get_uint64()
		self.data_trigger  = ManSettings.get_value(self.TRIGGER).get_uint32()
		self.ax.clear()
		self.ax.grid(True)
		
		Cursor = self.Conn.cursor()
		data = Cursor.execute("SELECT ref_id,data_changed,data_value,data_type FROM "+self.data_base+" WHERE data_creator = "+str(self.data_creator)+" and data_trigger = "+str(self.data_trigger))
		#print "selected "+str(len(data.rowcount))+" data"
		#if data.rowcount > 0 :
		#	print "selected "+str(data.rowcoun)+" data"
		r = data.fetchone()
		life_zero     = 0.0
		starti        = 0.0
		stopi         = 0.0
		is_zero       = False
		self.X = array('f',[])
		self.Y = array('f',[])
		print "search data... "+self.data_base+" data creator="+str(self.data_creator)+" data_trigger="+str(self.data_trigger)
		if r != None :
			print "Find data ... "
			self.start_time = r[1]
			print str(self.start_time) + "test " + str(self.start_time-r[1])
			for row in data:
				self.last_id=row[0]
				self.X.append(row[1]-self.start_time)
				self.Y.append(row[2])
				if row[3] == self.ULTIMATE_SIGNAL_JUSTIFICATED:
					life_zero = r[2]
					is_zero   = True
				if row[3] == self.ULTIMATE_SIGNAL_INTEGRATION_START:
					starti = row[1]-self.start_time
				if row[3] == self.ULTIMATE_SIGNAL_INTEGRATION_STOP:
					stopi  = row[1]-self.start_time
			
			print "Stop I = "+str(starti)+"Stop I = "+str(stopi)+" data ... "
			self.ax.plot(self.X, self.Y, color='blue', label='Signal', linewidth=2)
			self.ax.set_ylim(-0.05,1.0)
			self.ax.set_ylabel("Signal [fsr]")
			self.ax.set_xlabel("Time [sec]")
			if is_zero == True:
				l = self.ax.axhline(y=life_zero,linewidth=1, color='r')
			if starti > 0.0:
				lsi = self.ax.axvline(x=starti,linewidth=1, color='g')
			if stopi > 0.0:
				lsti = self.ax.axvline(x=stopi,linewidth=2, color='r')

		else :
			self.ax.plot(self.X, self.Y, color='blue', label='Signal', linewidth=2)
			self.ax.set_ylim(-0.05,1.0)
			self.ax.set_ylabel("Signal [fsr]")
			self.ax.set_xlabel("Time [sec]")


		self.canvas.draw()


	def changed_main_settings(self,setting,property):
		if property == 'signal-trigger':
			self.auto_update()
		elif property != 'plug-id':
			self.update()
		print "change settings property "+property

class MyPlugin(Gtk.Plug):
	def __init__(self):
		Gtk.Plug.__init__(self)
		Wid = 0L
		print "Plug ID=", self.get_id()
		def embed_event(widget):
			print "I (",widget,") have just been embedded!"
		Id = "PLUGID="+str(self.get_id())+"\n"
		sys.stdout.write(Id)
		ManSettings.set_value('plug-id', GLib.Variant('t',self.get_id()))
		ManSettings.apply()
		self.connect("embedded", embed_event)
		self.signal=Signal()
		sw = Gtk.ScrolledWindow()
		self.add(sw)
		sw.add_with_viewport(self.signal.canvas)

	def start_update(self):
		self.signal.update()


class MyWindow(Gtk.Window):
	def __init__(self, app):
		Gtk.Window.__init__(self, title="Plugin Example", application=app)
		self.signal=Signal()
		self.set_default_size(800, 600)
		#self.signal.draw()
		#self.signal.fig.canvas.mpl_connect('motion_notify_event', self.updatecursorposition)
		#self.signal.fig.canvas.mpl_connect('button_press_event', self.updatezoom)
		#
		sw = Gtk.ScrolledWindow()
		sw.add_with_viewport(self.signal.canvas)
		self.add(sw)
		self.connect("delete-event", Gtk.main_quit)


	def start_update(self):
		self.signal.update()



class MyApplication(Gtk.Application):
	def __init__(self):
		Gtk.Application.__init__(self,application_id="com.lar.tera.signal",flags=Gio.ApplicationFlags.FLAGS_NONE)
		self.win = None
		
	def do_activate(self):
		self.win = MyPlugin(self)
		self.win.show_all()
		

	def do_startup(self):
		Gtk.Application.do_startup(self)
		new_action = Gio.SimpleAction.new_stateful("new", GLib.VariantType.new('tu'), GLib.Variant.new_uint64('u'))
		new_action = Gio.SimpleAction.new("new", None)
        new_action.connect("activate", self.new_cb)
        # add the action to the application
        self.add_action(new_action)

		GLib.timeout_add_seconds(5, self.check_embedded)

	def check_embedded(self):
		if self.win.get_embedded() == False:
			self.quit()
		return True


print "PLUGID=123123123123"
sys.stdout.write("PLUGID=123123123123")
while True:
	#print "read stdin thread"
	buff = ''
	buff += sys.stdin.read(1)
	if buff.endswith('\n'):
		print buff[:-1]
		buff = ''

#win = MyPlugin()
#win.show_all()
#Gtk.main()
#SIGNAL_RUN = False

