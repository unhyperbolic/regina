
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  iOS User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2013, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  As an exception, when this program is distributed through (i) the     *
 *  App Store by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or     *
 *  (iii) Google Play by Google Inc., then that store may impose any      *
 *  digital rights management, device limits and/or redistribution        *
 *  restrictions that are required by its terms of service.               *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

#import "DetailViewController.h"
#import "MBProgressHUD.h"
#import "NewPacketController.h"
#import "NewPacketMenu.h"
#import "PacketListenerIOS.h"
#import "PacketManagerIOS.h"
#import "PacketTreeController.h"
#import "ReginaHelper.h"

#import "file/nxmlfile.h"
#import "packet/npacket.h"
#import "packet/ncontainer.h"
#import "packet/ntext.h"

@interface PacketTreeController () <UIAlertViewDelegate, PacketDelegate> {
    /**
     * Stores the immediate children of the subtree packet (self.node), in order.
     * This should always be in sync with the packets as they are stored in Regina's calculation engine.
     */
    NSPointerArray *_packets;

    /**
     * Stores the index of the packet with which the user last interacted.
     * This is simply used as a hint for packet-to-index lookup,
     * in the hope of avoiding a linear scan through the _packets array.
     *
     * It does not matter if this becomes out of date - the only possible consequence
     * is that packet-to-index lookup may run in linear time instead of constant time.
     */
    NSInteger _recentPacketIndex;

    /**
     * Listens on the subtree packet (self.node), as well as all of its immediate children.
     */
    PacketListenerIOS* _listener;

    /**
     * Stores the row of the table containing the "Browse subpackets" cell, or 0 if
     * this cell is not currently visible.
     */
    NSInteger _subtreeRow;
}

@end

@implementation PacketTreeController

- (void)newDocument
{
    NSLog(@"Creating new document...");
    
    [ReginaHelper detail].doc = [ReginaDocument documentNewWithCompletionHandler:^(ReginaDocument* doc) {
        if (! doc) {
            NSLog(@"Could not create.");
            return;
        }
        NSLog(@"Created.");
        _node = doc.tree;
        _listener = [PacketListenerIOS listenerWithPacket:_node delegate:self listenChildren:YES];
        self.title = doc.localizedName;
        [self refreshPackets];

        // Be helpful to the user and open the placeholder text packet that is
        // created with every new document.
        if (_node->getFirstTreeChild())
            ReginaHelper.detail.packet = _node->getFirstTreeChild();
    }];
}

- (void)openDocument:(ReginaDocument*)doc
{
    NSLog(@"Opening document...");
    
    // We use an activity indicator since files could take some time to load.
    UIView* rootView = [UIApplication sharedApplication].keyWindow.rootViewController.view;
    MBProgressHUD* hud = [MBProgressHUD showHUDAddedTo:rootView animated:YES];
    [hud setLabelText:@"Loading"];
    
    [doc openWithCompletionHandler:^(BOOL success) {
        [MBProgressHUD hideHUDForView:rootView animated:YES];
        if (! success) {
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Could Not Open Document"
                                                            message:@"I can read Regina's own data files, as well as SnapPea/SnapPy triangulation files.  Please ensure that your document uses one of these file formats."
                                                           delegate:self
                                                  cancelButtonTitle:@"Close"
                                                  otherButtonTitles:nil];
            
            // Ensure that this tree controller is popped when the alert is dismissed.
            alert.delegate = self;
            
            [alert show];
            return;
        }
        NSLog(@"Opened.");
        _node = doc.tree;
        _listener = [PacketListenerIOS listenerWithPacket:_node delegate:self listenChildren:YES];
        self.title = doc.localizedName;
        [self refreshPackets];

        // If there is only one packet, open it immediately.
        if (_node->getFirstTreeChild() && ! _node->getFirstTreeChild()->getNextTreeSibling())
            ReginaHelper.detail.packet = _node->getFirstTreeChild();
    }];
    
    [ReginaHelper detail].doc = doc;
}

- (void)openSubtree:(regina::NPacket *)p
{
    _node = p;
    _listener = [PacketListenerIOS listenerWithPacket:_node delegate:self listenChildren:YES];
    self.title = [NSString stringWithUTF8String:p->getPacketLabel().c_str()];
    [self refreshPackets];
}

- (void)refreshPackets {
    if (! _node)
        return;

    _packets = [NSPointerArray pointerArrayWithOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsOpaquePersonality];
    regina::NPacket* p;
    for (p = _node->getFirstTreeChild(); p; p = p->getNextTreeSibling())
        [_packets addPointer:p];

    [self.tableView reloadData];
}

- (int)packetIndexForPath:(NSIndexPath*)indexPath {
    if (_subtreeRow <= indexPath.row && _subtreeRow > 0)
        return indexPath.row - 1;
    else
        return indexPath.row;
}

// Note: if indexPath points to the subtree row, this returns the associated packet.
- (regina::NPacket*)packetForPath:(NSIndexPath*)indexPath {
    if (_subtreeRow <= indexPath.row && _subtreeRow > 0)
        return static_cast<regina::NPacket*>([_packets pointerAtIndex:(indexPath.row - 1)]);
    else
        return static_cast<regina::NPacket*>([_packets pointerAtIndex:indexPath.row]);
}

- (int)packetIndexForPacket:(regina::NPacket*)packet {
    if (_recentPacketIndex < _packets.count && [_packets pointerAtIndex:_recentPacketIndex] == packet)
        return _recentPacketIndex;

    // Note: it is possible that the array is not in sync with the packet tree.
    // (This happens for instance when a new packet is created in the middle
    // of the list as opposed to the end.)
    NSLog(@"Performing linear search for packet.");
    int index = 0;
    for ( ; index < _packets.count; ++index) {
        if ([_packets pointerAtIndex:index] == packet) {
            _recentPacketIndex = index;
            return index;
        }
    }
    return -1;
}

- (NSIndexPath*)pathForPacket:(regina::NPacket*)packet {
    int index = [self packetIndexForPacket:packet];
    if (index < 0)
        return nil;
    if (_subtreeRow == 0 || _subtreeRow > index)
        return [NSIndexPath indexPathForRow:index inSection:0];
    else
        return [NSIndexPath indexPathForRow:(index + 1) inSection:0];
}

- (NSIndexPath*)pathForPacketIndex:(int)index {
    if (_subtreeRow == 0 || _subtreeRow > index)
        return [NSIndexPath indexPathForRow:index inSection:0];
    else
        return [NSIndexPath indexPathForRow:(index + 1) inSection:0];
}

- (BOOL)selectPacket:(regina::NPacket*)p {
    if (p->getTreeParent() != self.node)
        return NO;

    // This packet should be in our list of immediate children.
    // If it is a container then we don't select it (since pressing on
    // a container cell would normally push immediately to the subtree).
    // Regardless, however, we ensure that the cell is visible on screen.
    NSIndexPath* path = [self pathForPacket:p];
    if (! path)
        return NO;

    if (p->getPacketType() != regina::PACKET_CONTAINER && ! [self.tableView.indexPathForSelectedRow isEqual:path])
        [self.tableView selectRowAtIndexPath:path animated:NO scrollPosition:UITableViewScrollPositionNone];

    [self.tableView scrollToRowAtIndexPath:[self pathForPacket:p] atScrollPosition:UITableViewScrollPositionNone animated:YES];
    return YES;
}

- (void)newPacket:(regina::PacketType)type {
    NewPacketSpec* spec = [NewPacketSpec specWithType:type tree:self];
    if (! [spec hasParentWithAlert])
        return;

    [PacketManagerIOS newPacket:spec];
}

- (void)navigateToPacket:(regina::NPacket *)dest
{
    if ((! dest) || dest == self.node)
        return;
    
    if (dest->getTreeParent() == self.node) {
        // This involves a single push.
        PacketTreeController* subtree = [self.storyboard instantiateViewControllerWithIdentifier:@"packetTree"];
        [subtree openSubtree:dest];
        [self.navigationController pushViewController:subtree animated:YES];
    } else if (self.node->getTreeParent() == dest) {
        // This involves a single pop.
        [self.navigationController popViewControllerAnimated:YES];
    } else {
        // Something more complicated.
        NSPointerArray* myLeafToRoot = [NSPointerArray pointerArrayWithOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsOpaquePersonality];
        NSPointerArray* destLeafToRoot = [NSPointerArray pointerArrayWithOptions:NSPointerFunctionsOpaqueMemory | NSPointerFunctionsOpaquePersonality];

        regina::NPacket* p;
        for (p = self.node; p; p = p->getTreeParent())
            [myLeafToRoot addPointer:p];
        for (p = dest; p; p = p->getTreeParent())
            [destLeafToRoot addPointer:p];

        long myIdx = myLeafToRoot.count - 1; // Root.
        long destIdx = destLeafToRoot.count - 1; // Root.
        while (true) {
            // INV: myLeafToRoot[myIdx,...] == destLeafToRoot[destIdx,...].
            if (myIdx == 0 || destIdx == 0)
                break;
            if ([myLeafToRoot pointerAtIndex:(myIdx-1)] != [destLeafToRoot pointerAtIndex:(destIdx-1)])
                break;
            --myIdx;
            --destIdx;
        }

        regina::NPacket* lca = static_cast<regina::NPacket*>([destLeafToRoot pointerAtIndex:destIdx]);

        NSMutableArray* controllers = [[NSMutableArray alloc] init];
        long i;
        PacketTreeController* subtree;
        UIViewController* c;
        for (i = 0; ; ++i) {
            c = self.navigationController.viewControllers[i];
            [controllers addObject:c];
            if ([c isKindOfClass:[PacketTreeController class]] && static_cast<PacketTreeController*>(c).node == lca)
                break;
        }
        for (i = destIdx - 1; i >= 0; --i) {
            subtree = [self.storyboard instantiateViewControllerWithIdentifier:@"packetTree"];
            [subtree openSubtree:static_cast<regina::NPacket*>([destLeafToRoot pointerAtIndex:i])];
            [controllers addObject:subtree];
        }

        [self.navigationController setViewControllers:controllers animated:YES];
    }
}

- (void)dealloc
{
    [_listener permanentlyUnlisten];
}

#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([[segue identifier] isEqualToString:@"openSubtree"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        [[segue destinationViewController] openSubtree:[self packetForPath:indexPath]];
    } else if ([[segue identifier] isEqualToString:@"newPacket"]) {
        NewPacketMenu* menu = static_cast<NewPacketMenu*>(segue.destinationViewController);
        menu.packetTree = self;
        menu.seguePopoverController = static_cast<UIStoryboardPopoverSegue*>(segue).popoverController;
    }
}

#pragma mark - Packet listener

- (void)packetWasRenamed:(regina::NPacket *)packet {
    [[ReginaHelper document] setDirty];
    if (packet == self.node) {
        // Refresh the title, but only if this is not the root of the packet tree.
        if (packet->getTreeParent())
            self.title = [NSString stringWithUTF8String:packet->getPacketLabel().c_str()];
    } else {
        // Don't animate, since this was most likely a result of the user editing the cell.
        NSIndexPath* path = [self pathForPacket:packet];
        if (path)
            [self.tableView reloadRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationNone];
    }
}

- (void)childWasAddedTo:(regina::NPacket*)packet child:(regina::NPacket*)child {
    [[ReginaHelper document] setDirty];

    NSIndexPath* path;
    if (packet == self.node) {
        // We have a new row for our table of children.
        int childIndex;
        if (! child->getNextTreeSibling()) {
            // The new row goes on the end of the list.
            childIndex = _packets.count;
            [_packets addPointer:child];
        } else {
            // This requires a linear scan to find childIndex, but the good news
            // is that the linear scan only happens if the insertion is *not* at
            // the end of the list.
            childIndex = [self packetIndexForPacket:child->getNextTreeSibling()];
            [_packets insertPointer:child atIndex:childIndex];
        }

        _recentPacketIndex = childIndex;
        path = [self pathForPacketIndex:childIndex];
        [self.tableView insertRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationAutomatic];
    } else {
        // One of our children needs a new subtitle (which counts *its* children).
        path = [self pathForPacket:child->getTreeParent()];
        if (path)
            [self.tableView reloadRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationAutomatic];
    }

    // Disable the following code, since the new packet actions will call selectPacket:,
    // which scrolls the table anyway.
    /*
    if (path)
        [self.tableView scrollToRowAtIndexPath:path atScrollPosition:UITableViewScrollPositionNone animated:YES];
     */
}

- (void)childWasRemovedFrom:(regina::NPacket *)packet child:(regina::NPacket *)child inParentDestructor:(bool)destructor {
    if (destructor)
        return;

    if (packet == self.node) {
        // No need to update the table, since this action can only have happened as a result
        // of user interaction with the table.
        [[ReginaHelper document] setDirty];
    } else {
        // One of our children needs a new subtitle (which counts *its* children).
        NSIndexPath* path = [self pathForPacket:packet];
        if (path)
            [self.tableView reloadRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

- (void)childrenWereReordered:(regina::NPacket *)packet {
    // No need to update the table, since this action can only have happened as a result
    // of user interaction with the table.
    [[ReginaHelper document] setDirty];
}

#pragma mark - Editable table view

- (BOOL)renameAllowed:(NSIndexPath *)path
{
    return ! (_subtreeRow > 0 && _subtreeRow == path.row);
}

- (NSString *)renameInit:(NSIndexPath *)path
{
    return [NSString stringWithUTF8String:[self packetForPath:path]->getPacketLabel().c_str()];
}

- (void)renameDone:(NSIndexPath *)path result:(NSString *)result
{
    _recentPacketIndex = [self packetIndexForPath:path];
    regina::NPacket* p = static_cast<regina::NPacket*>([_packets pointerAtIndex:_recentPacketIndex]);
    p->setPacketLabel([[result stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] UTF8String]);
}

- (NSArray *)otherActionLabels
{
    _recentPacketIndex = [self packetIndexForPath:self.actionPath];
    regina::NPacket* p = static_cast<regina::NPacket*>([_packets pointerAtIndex:_recentPacketIndex]);

    if (p->getFirstTreeChild())
        return @[@"Clone", @"Clone Subtree"];
    else
        return @[@"Clone"];
}

- (void)otherActionSelected:(int)which
{
    _recentPacketIndex = [self packetIndexForPath:self.actionPath];
    regina::NPacket* p = static_cast<regina::NPacket*>([_packets pointerAtIndex:_recentPacketIndex]);

    if (which == 0) {
        /* Clone packet */
        p->clone(false, false);
    } else if (which == 1 && p->getFirstTreeChild()) {
        /* Clone subtree */
        p->clone(true, false);
    }
}

- (NSString *)deleteConfirmation:(NSIndexPath *)path
{
    _recentPacketIndex = [self packetIndexForPath:path];
    regina::NPacket* p = static_cast<regina::NPacket*>([_packets pointerAtIndex:_recentPacketIndex]);

    unsigned long totalPackets = p->getNumberOfDescendants();
    if (totalPackets == 0)
        return @"Delete packet";
    else
        return [NSString stringWithFormat:@"Delete %ld packets", totalPackets + 1];
}

- (void)deleteAction
{
    int packetIndex = [self packetIndexForPath:self.actionPath];
    regina::NPacket* packet = static_cast<regina::NPacket*>([_packets pointerAtIndex:packetIndex]);

    [_packets removePointerAtIndex:packetIndex];

    if (_subtreeRow == self.actionPath.row + 1) {
        // We need to remove the subtree cell also.
        NSIndexPath* subtreePath = [NSIndexPath indexPathForRow:_subtreeRow inSection:0];
        _subtreeRow = 0;
        [self.tableView deleteRowsAtIndexPaths:@[self.actionPath, subtreePath]
                              withRowAnimation:UITableViewRowAnimationFade];
    } else {
        [self.tableView deleteRowsAtIndexPaths:@[self.actionPath]
                              withRowAnimation:UITableViewRowAnimationFade];
    }

    delete packet;
}

#pragma mark - Table view

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return (_subtreeRow > 0 ? [_packets count] + 1 : [_packets count]);
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (_subtreeRow > 0 && indexPath.row == _subtreeRow)
        return [tableView dequeueReusableCellWithIdentifier:@"Subtree" forIndexPath:indexPath];

    regina::NPacket* p = [self packetForPath:indexPath];

    UITableViewCell *cell;
    if (p->getPacketType() == regina::NContainer::packetType)
        cell = [tableView dequeueReusableCellWithIdentifier:@"Container" forIndexPath:indexPath];
    else
        cell = [tableView dequeueReusableCellWithIdentifier:@"Packet" forIndexPath:indexPath];
    cell.textLabel.text = [NSString stringWithUTF8String:p->getPacketLabel().c_str()];
    unsigned long sub = p->getNumberOfChildren();
    if (sub == 0)
        cell.detailTextLabel.text = @"";
    else if (sub == 1)
        cell.detailTextLabel.text = @"1 subpacket";
    else
        cell.detailTextLabel.text = [NSString stringWithFormat:@"%lu subpackets", sub];
    cell.imageView.image = [PacketManagerIOS iconFor:p];

    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return (! self.actionPath) && (! (_subtreeRow > 0 && indexPath.row == _subtreeRow));
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if (_subtreeRow > 0 && indexPath.row == _subtreeRow) {
        // The user has selected the existing browse-subtree cell.
        // This is already attached to its own segue.
        // Nothing more to do here.
        return;
    }

    _recentPacketIndex = [self packetIndexForPath:indexPath];
    regina::NPacket* p = static_cast<regina::NPacket*>([_packets pointerAtIndex:_recentPacketIndex]);

    if (_subtreeRow != indexPath.row + 1) {
        // The subtree row might need to change.
        NSInteger oldSubtreeRow = _subtreeRow;

        if (p->getPacketType() != regina::NContainer::packetType && p->getFirstTreeChild()) {
            // The subtree row should appear beneath this packet.
            _subtreeRow = _recentPacketIndex + 1;
            if (oldSubtreeRow == 0) {
                [tableView insertRowsAtIndexPaths:[NSArray arrayWithObject:
                                                   [NSIndexPath indexPathForRow:_subtreeRow inSection:0]]
                                 withRowAnimation:UITableViewRowAnimationTop];
            } else {
                [tableView beginUpdates];
                [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:
                                                   [NSIndexPath indexPathForRow:oldSubtreeRow inSection:0]]
                                 withRowAnimation:UITableViewRowAnimationTop];
                [tableView insertRowsAtIndexPaths:[NSArray arrayWithObject:
                                                   [NSIndexPath indexPathForRow:_subtreeRow inSection:0]]
                                 withRowAnimation:UITableViewRowAnimationTop];
                [tableView endUpdates];
            }
        } else {
            // The subtree row should disappear, since we do not offer it for the selected packet.
            _subtreeRow = 0;
            if (oldSubtreeRow > 0)
                [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:
                                                   [NSIndexPath indexPathForRow:oldSubtreeRow inSection:0]]
                                 withRowAnimation:UITableViewRowAnimationTop];
        }
    }

    if (p->getPacketType() != regina::PACKET_CONTAINER)
        [ReginaHelper detail].packet = p;
}

#pragma mark - Alert view

- (void)alertView:(UIAlertView *)alertView willDismissWithButtonIndex:(NSInteger)buttonIndex
{
    // This is a consequence of failing to open a document.
    // Specifically, this is called after the corresponding "could not open" alert is dismissed.
    // Pop back to the main documents list.
    
    // We put the call to popViewControllerAnimated: here, after the alert is dismissed,
    // so that the forward animation that pushed this packet tree controller has time to finish
    // and we can thus avoid errors from nested push/pop segues.
    [self.navigationController popViewControllerAnimated:YES];
}

@end