/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.android.framebufferizer.utils;

import com.android.framebufferizer.NativeRenderer;

import java.awt.*;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBufferInt;
import java.awt.image.DirectColorModel;
import java.awt.image.Raster;
import java.awt.image.RenderedImage;
import java.awt.image.WritableRaster;

import javax.swing.*;


public class FrameBufferBuffer extends JPanel implements ComponentListener, MouseMotionListener {
    public class MagnifiedView extends JPanel implements ComponentListener {
        private BufferedImage mImage;

        protected MagnifiedView() {
            addComponentListener(this);
        }

        @Override
        public void componentResized(ComponentEvent e) {
            synchronized (this) {
                mImage = new BufferedImage(getWidth(), getHeight(), BufferedImage.TYPE_INT_ARGB_PRE);
            }
        }

        @Override
        public void componentMoved(ComponentEvent e) {

        }

        @Override
        public void componentShown(ComponentEvent e) {

        }

        @Override
        public void componentHidden(ComponentEvent e) {

        }

        public int d() { return getWidth()/ 5; }

        public void draw(RenderedImage image) {
            if (mImage == null) return;
            Graphics2D gc = mImage.createGraphics();
            gc.drawRenderedImage(image, AffineTransform.getScaleInstance(5.0, 5.0));
            repaint();
        }

        @Override
        public void paint(Graphics g) {
            synchronized (this) {
                g.drawImage(mImage, 0, 0, null);
            }
        }

    }

    private BufferedImage mImage;
    private DataBufferInt mBuffer;
    private MagnifiedView mMagnifiedView;

    public MagnifiedView getMagnifiedView() {
        if (mMagnifiedView == null) {
            mMagnifiedView = new MagnifiedView();
        }
        return mMagnifiedView;
    }

    @Override
    public void mouseDragged(MouseEvent e) {

    }

    @Override
    public void mouseMoved(MouseEvent e) {
        if (mMagnifiedView != null) {
            final int rMask = 0xff;
            final int gMask = 0xff00;
            final int bMask = 0xff0000;
            final int bpp = 24;
            int d = mMagnifiedView.d();
            int x = e.getX() - d/2;
            if (x + d > mImage.getWidth()) {
                x = mImage.getWidth() - d;
            }
            if (x < 0) { x = 0; }
            int y = e.getY() - d/2;
            if (y + d > mImage.getHeight()) {
                y = mImage.getHeight() - d;
            }
            if (y < 0) { y = 0; }
            if (mImage.getWidth() < d) {
                d = mImage.getWidth();
            }
            if (mImage.getHeight() < d) {
                d = mImage.getHeight();
            }
            mMagnifiedView.draw(mImage.getSubimage(x, y, d, d));
        }
    }


    protected FrameBufferBuffer(BufferedImage image, DataBufferInt buffer) {
        mImage = image;
        mBuffer = buffer;
        addComponentListener(this);
        addMouseMotionListener(this);
    }

    public static FrameBufferBuffer createFrameBufferBuffer(int w, int h, int linestride) {
        final int rMask = 0xff;
        final int gMask = 0xff00;
        final int bMask = 0xff0000;
        final int bpp = 24;
        DataBufferInt dataBuffer = new DataBufferInt(h * linestride);
        WritableRaster raster = Raster.createPackedRaster(dataBuffer, w, h, linestride,
                new int[]{rMask, gMask, bMask}, null);
        ColorModel colorModel = new DirectColorModel(bpp, rMask, gMask, bMask);
        BufferedImage image = new BufferedImage(colorModel, raster, true, null);
        NativeRenderer.setDeviceInfo(w, h, -1, 3.5, 5.5);
        int error = NativeRenderer.renderBuffer(0, 0, w, h, linestride, dataBuffer.getData());
        if (error != 0) {
            System.out.println("Error rendering native buffer " + error);
        }
        return new FrameBufferBuffer(image, dataBuffer);
    }

    public BufferedImage getImage() {
        return mImage;
    }

    public DataBufferInt getBuffer() {
        return mBuffer;
    }

    @Override
    public void componentResized(ComponentEvent e) {
        int w = getWidth();
        int h = getHeight();
        final int linestride = w;
        final int rMask = 0xff;
        final int gMask = 0xff00;
        final int bMask = 0xff0000;
        final int bpp = 24;
        synchronized (this) {
            mBuffer = new DataBufferInt(h * linestride);
            WritableRaster raster = Raster.createPackedRaster(mBuffer, w, h, linestride,
                    new int[]{rMask, gMask, bMask}, null);
            ColorModel colorModel = new DirectColorModel(bpp, rMask, gMask, bMask);
            mImage = new BufferedImage(colorModel, raster, true, null);
            NativeRenderer.setDeviceInfo(w, h, -1, w/412.0, 5.5);
            int error = NativeRenderer.renderBuffer(0, 0, w, h, linestride, mBuffer.getData());
            if (error != 0) {
                System.out.println("Error rendering native buffer " + error);
            }
        }
        repaint();
    }

    @Override
    public void componentMoved(ComponentEvent e) {

    }

    @Override
    public void componentShown(ComponentEvent e) {

    }

    @Override
    public void componentHidden(ComponentEvent e) {

    }

    @Override
    public void paint(Graphics g) {
        synchronized (this) {
            g.drawImage(mImage, 0, 0, null);
        }
    }
}
