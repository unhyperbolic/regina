
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  iOS User Interface                                                    *
 *                                                                        *
 *  Copyright (c) 1999-2016, Ben Burton                                   *
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

#import "PacketPicker.h"
#import "packet/npacket.h"

#pragma mark - Packet choice

@interface PacketChoice : NSObject

@property (assign, nonatomic, readonly) regina::NPacket* packet;
@property (strong, nonatomic, readonly) NSString* text;

- (id)initWithPacket:(regina::NPacket*)p;
- (id)initEmptyWithText:(NSString*)text;
+ (id)packetChoiceWithPacket:(regina::NPacket*)p;
+ (id)packetChoiceEmptyWithText:(NSString*)text;

@end

@implementation PacketChoice

- (id)initWithPacket:(regina::NPacket*)p {
    self = [super init];
    if (self) {
        _packet = p;
        _text = [NSString stringWithUTF8String:p->label().c_str()];
    }
    return self;
}

- (id)initEmptyWithText:(NSString *)text
{
    self = [super init];
    if (self) {
        _packet = 0;
        _text = text;
    }
    return self;
}

+ (id)packetChoiceWithPacket:(regina::NPacket*)p {
    return [[PacketChoice alloc] initWithPacket:p];
}

+ (id)packetChoiceEmptyWithText:(NSString *)text {
    return [[PacketChoice alloc] initEmptyWithText:text];
}

@end

#pragma mark - Packet picker

@interface PacketPicker () <UIPickerViewDataSource, UIPickerViewDelegate>

@property (strong, nonatomic) NSMutableArray* choices;
@property (assign, nonatomic) BOOL allowNone; // Always NO for now.

@end

@implementation PacketPicker

- (void)fill:(regina::NPacket *)tree type:(regina::PacketType)packetType allowNone:(BOOL)allowNone noneText:(NSString *)noneText
{
    self.allowNone = allowNone;

    self.choices = [NSMutableArray array];
    if (allowNone)
        [self.choices addObject:[PacketChoice packetChoiceEmptyWithText:noneText]];

    regina::NPacket* p;
    for (p = tree; p; p = p->nextTreePacket())
        if (p->type() == packetType)
            [self.choices addObject:[PacketChoice packetChoiceWithPacket:p]];

    if (self.choices.count == 0)
        [self.choices addObject:[PacketChoice packetChoiceEmptyWithText:noneText]];

    self.delegate = self;
    self.dataSource = self;
}

- (BOOL)empty
{
    return ((! self.allowNone) && (self.choices.count == 1) && (! [self.choices[0] packet]));
}

- (regina::NPacket *)selectedPacket
{
    return [self.choices[[self selectedRowInComponent:0]] packet];
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return self.choices.count;
}

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    return [self.choices[row] text];
}

@end
