
/**************************************************************************
 *                                                                        *
 *  Regina - A normal surface theory calculator                           *
 *  Java user interface                                                   *
 *                                                                        *
 *  Copyright (c) 1999-2001, Ben Burton                                   *
 *  For further details contact Ben Burton (benb@acm.org).                *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,        *
 *  MA 02111-1307, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

package normal.packetui.packet;

import java.awt.*;
import javax.swing.*;
import normal.Shell;
import normal.engine.packet.*;
import normal.packetfilter.*;
import normal.packetui.*;

/**
 * An interface through which a user can create a new container packet.
 * See <tt>PacketCreator</tt> for more details.
 *
 * @see normal.packetui.PacketCreator
 */
public class NContainerCreator extends JPanel implements PacketCreator {
    /**
     * Create the new interface.
     */
    public NContainerCreator() {
        init();
    }

    /**
     * Create the interface's internal components.
     */
    private void init() {
        // Set the layout manager.
        setLayout(new BorderLayout());

        // Insert the components.
        add(new JLabel("A new container will be created for you."),
            BorderLayout.CENTER);
    }

    public NPacket makeNewPacket(Shell shell, NPacket parent,
            Dialog parentDialog) {
        return shell.getEngine().newNContainer();
    }

    public Component getCreator() {
        return this;
    }

    public String getDialogTitle() {
        return "New Container";
    }

    public String getSuggestedPacketLabel() {
        return "New Container";
    }

    public PacketFilter getPacketFilter() {
        return AllowAll.instance;
    }
}
