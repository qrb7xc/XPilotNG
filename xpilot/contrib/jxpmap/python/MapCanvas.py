import gtk
import GDK

from javastuff import *

class MouseEvent:
    def __init__(self, point, button, area, orig = None):
        self.p = point
        self.button = button
        self.area = area
        self.orig = orig

    def getX(self):
        return self.p.x

    def getY(self):
        return self.p.y

    def getPoint(self):
        return self.p

    def getSource(self):
        return self.area

class DrawInfo:
    pass

class MapCanvas:
    def __init__(self, area):
        self.eventHandler = None
        self.it = None
        self.at = None
        self.posx = 0
        self.posy = 0
        self.area = area
        self.erase = 0
        gc = area.get_window().new_gc()
        gc.function = GDK.XOR
        gc.foreground = area.get_colormap().alloc(65535, 65535, 65535)
        gc.line_style = GDK.LINE_ON_OFF_DASH
        self.previewgc = gc
        self.resize()

    def getModel(self):
        return self.model

    def setModel(self, m):
        self.model = m
        self.scale = m.getDefaultScale()
        self.it = None
        self.at = None
        self.repaint()

    def getScale(self):
        return self.scale

    def setScale(self, s):
        self.scale = s
        self.it = None
        self.at = None

    def resize(self):
        widget = self.area
        x, y, w, h = widget.get_allocation()
        self.scratch = gtk.create_pixmap(widget.get_window(), w, h, -1)

    # Set in screen coordinates
    def setCenter(self, x, y):
        p = self.it(Point(x, y))
        self.posx = p.x % self.model.options.size.width
        self.posy = p.y % self.model.options.size.height

    def isErase(self):
        return self.erase

    def setErase(self, erase):
        self.erase = erase

    def setCanvasEventHandler(self, h):
        self.eventHandler = h

    def getCanvasEventHandler(self):
        return self.eventHandler

    def wrapdist2(self, p1, p2):
        mapSize = self.model.options.size
        dx = (p1.x - p2.x) % mapSize.width
        if dx > mapSize.width / 2:
            dx -= mapSize.width
        dy = (p1.y - p2.y) % mapSize.height
        if dy > mapSize.height / 2:
            dy -= mapSize.height
        return float(dx)**2 + float(dy)**2

    def wrap(self, p1, p2):
        s = self.model.options.size
        r= (int(round((p2.x - p1.x + .0) / s.width)) * s.width,
                int(round((p2.y - p1.y + .0) / s.height)) * s.height)
        return r

    def createTransforms(self, width, height):
        self.at = AffineTransform()
        self.at.scale(self.scale, -self.scale)
        self.at.translate(width / self.scale / 2, -height / self.scale / 2)
        self.at.translate(-self.posx, -self.posy)
        self.it = self.at.createInverse()

    def getInverse(self):
        return self.it

    def getTransform(self):
        return self.at

    def wrapOffsets(self, view, b):
        map = self.model.options.size
        nx = (view.x - (b.x + b.width)) / map.width + 1
        xx = (view.x + view.width - b.x) / map.width + 1
        ny = (view.y - (b.y + b.height)) / map.height + 1
        xy = (view.y + view.height - b.y) / map.height + 1
        return [(x * map.width, y * map.height) for x in range(nx, xx)\
                for y in range(ny, xy)]

    def repaint(self):
        self.paint(None)

    def paint(self, region):
        area = self.area
        gc = area.get_window().new_gc()
        colormap = area.get_colormap()
        gc.foreground = colormap.alloc(0, 0, 0)
        _, _, aw, ah = area.get_allocation()
        if region:
            x, y, w, h = region
        else:
            w, h = aw, ah
            x = y = 0
        self.createTransforms(aw, ah)
        gtk.draw_rectangle(self.scratch, gc, gtk.TRUE, x, y, w, h)
        mapSize = self.model.options.size
        inv = self.getInverse()
        if region:
            upleft = Point(x, y + h)
            downright = Point(x + w, y)
        else:
            upleft = Point(0, ah)
            downright = Point(aw, 0)
        min = inv(upleft)
        max = inv(downright)
        view = Rectangle(min.x, min.y, max.x - min.x, max.y - min.y)
        self.view = view
        os = self.model.objects[:]
        os.reverse()
        di = DrawInfo()
        di.gc = gc
        di.area = self.scratch
        di.scale = self.scale
        di.colormap = colormap
        di.canvas = self
        tx2 = self.getTransform()
        for o in os:
            for wrap in self.wrapOffsets(view, o.getBounds()):
                tx = tx2.copy()
                tx.translate(*wrap)
                di.tx = tx
                o.paint(di)
        area.draw_pixmap(area.get_style().fg_gc[gtk.STATE_NORMAL],
                         self.scratch, x, y, x, y, w, h)
        return gtk.TRUE

    def handle(self, area, event):
        if self.model == None:
            return
        if self.eventHandler:
            if event.type == GDK.BUTTON_PRESS:
                evt = MouseEvent(self.it(Point(event.x, event.y)),
                                 event.button, self, event)
                print 'press', evt.p.x, evt.p.y
                self.eventHandler.mousePressed(evt)
            elif event.type == GDK.BUTTON_RELEASE:
                evt = MouseEvent(self.it(Point(event.x, event.y)),
                                 event.button, self, event)
                self.eventHandler.mouseReleased(evt)
            elif event.type == GDK.MOTION_NOTIFY:
                if event.is_hint:
                    x, y = event.window.pointer
                else:
                    x = event.x
                    y = event.y
                evt = MouseEvent(self.it(Point(x, y)), None, self, event)
                self.eventHandler.mouseMoved(evt)
        elif event.type == GDK.BUTTON_PRESS:
            evt = MouseEvent(self.it(Point(event.x, event.y)), event.button,
                             self, event)
            print 'press', evt.p.x, evt.p.y
            for o in self.model.objects:
                if o.checkEvent(self, evt):
                    return

    def drawShape(self, shape, gc = None, target = None):
        if gc is None:
            gc = self.previewgc
        if target is None:
            target = self.area.get_window()
            # Got use use the get_window() because gtk.draw_ methods don't
            # work on a DrawingArea instance, and the paint() methods can't
            # use target.draw_ as they need to work on Pixmaps too and those
            # don't have such methods.
        di = DrawInfo()
        di.gc = gc
        di.area = target
        tx = self.getTransform()
        min = Point()
        max = Point()
        mapSize = self.model.options.size
        for wrap in self.wrapOffsets(self.view, shape.getBounds()):
            di.tx = tx.copy()
            di.tx.translate(*wrap)
            shape.paint(di)
