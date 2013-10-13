
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

#import "PacketTreeController.h"
#import "file/nxmlfile.h"
#import "packet/npacket.h"
#import "packet/ncontainer.h"

@interface PacketTreeRow : NSObject

@property (assign, nonatomic, readonly) regina::NPacket* packet;
@property (assign, nonatomic, readonly) bool subtree;

- (id)initWithPacket:(regina::NPacket*)p subtree:(bool)s;
+ (id)packetTreeRowWithPacket:(regina::NPacket*)p subtree:(bool) s;

@end

@implementation PacketTreeRow

- (id)initWithPacket:(regina::NPacket*)p subtree:(bool)s {
    self = [super init];
    if (self) {
        _packet = p;
        _subtree = s;
    }
    return self;
}

+ (id)packetTreeRowWithPacket:(regina::NPacket*)p subtree:(bool) s {
    return [[PacketTreeRow alloc] initWithPacket:p subtree:s];
}

@end

@interface PacketTreeController () {
    NSMutableArray *_rows;
}

@property (assign, nonatomic) regina::NPacket* tree;
@property (assign, nonatomic) regina::NPacket* node;
@property (assign, nonatomic) bool example;
@property (strong, nonatomic) NSString* filename;

- (bool)loadTreeResource:(NSString*)filename;
- (void)configureView;
- (void)fill;

@end

@implementation PacketTreeController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self configureView];
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    if (! (_node && _node->getTreeParent())) {
        delete _tree;
        NSLog(@"Closing file and deleting from memory.");
    }
}

- (void)openExample:(Example*)e {
    _example = true;
    _filename = e.desc;
    [self loadTreeResource:e.file];
    // TODO: Trap errors.
}

- (bool)loadTreeResource:(NSString*)filename {
    NSBundle* mainBundle = [NSBundle mainBundle];
    if (! mainBundle) {
        NSLog(@"Could not access main bundle.");
        return false;
    }
    
    NSString* path = [mainBundle pathForResource:filename ofType:nil inDirectory:@"examples"];
    if (! path) {
        NSLog(@"Could not find resource: %@", filename);
        return false;
    }
    
    _tree = regina::readXMLFile([path UTF8String]);
    if (! _tree) {
        NSLog(@"Failed to read data file: %@", path);
        return false;
    }

    NSLog(@"Loaded file: %@", filename);
    _node = _tree;
    return true;
}

- (void)configureView {
    [self setTitle:_filename];
}

- (void)fill {
    if (! _node)
        return;
    
    _rows = [NSMutableArray array];
    
    regina::NPacket* p;
    for (p = _node->getFirstTreeChild(); p; p = p->getNextTreeSibling()) {
        if (p->getPacketType() == regina::NContainer::packetType) {
            [_rows addObject:[PacketTreeRow packetTreeRowWithPacket:p subtree:true]];
        } else {
            [_rows addObject:[PacketTreeRow packetTreeRowWithPacket:p subtree:false]];
            if (p->getFirstTreeChild()) {
                [_rows addObject:[PacketTreeRow packetTreeRowWithPacket:p subtree:true]];
            }
        }
    }
}

- (void)refreshPackets {
    [self fill];
    [self.tableView reloadData];
}

#pragma mark - Table View

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [_rows count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell;
    
    PacketTreeRow* r = _rows[indexPath.row];
    if (r.subtree) {
        cell = [tableView dequeueReusableCellWithIdentifier:@"Subtree" forIndexPath:indexPath];
        cell.textLabel.text = [NSString stringWithUTF8String:[r packet]->getPacketLabel().c_str()];
        cell.detailTextLabel.text = [NSString stringWithFormat:@"%lu subpackets", [r packet]->getNumberOfDescendants()];
    } else {
        cell = [tableView dequeueReusableCellWithIdentifier:@"Packet" forIndexPath:indexPath];
        cell.textLabel.text = [NSString stringWithUTF8String:[r packet]->getPacketLabel().c_str()];
        cell.detailTextLabel.text = [NSString stringWithUTF8String:[r packet]->getPacketTypeName().c_str()];
        switch ([r packet]->getPacketType()) {
            case regina::PACKET_ANGLESTRUCTURELIST:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/angles-32"];
                break;
            case regina::PACKET_CONTAINER:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/container-32"];
                break;
            case regina::PACKET_DIM2TRIANGULATION:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/dim2triangulation-32"];
                break;
            case regina::PACKET_NORMALSURFACELIST:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/surfaces-32"];
                break;
            case regina::PACKET_PDF:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/pdf-32"];
                break;
            case regina::PACKET_SCRIPT:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/script-32"];
                break;
            case regina::PACKET_SURFACEFILTER:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/filter-32"];
                break;
            case regina::PACKET_TEXT:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/text-32"];
                break;
            case regina::PACKET_TRIANGULATION:
                cell.imageView.image = [UIImage imageNamed:@"icons/packet/triangulation-32"];
                break;
        }
    }
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return NO;
}

@end
