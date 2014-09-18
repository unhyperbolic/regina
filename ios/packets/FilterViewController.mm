
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

#import "PacketManagerIOS.h"
#import "FilterViewController.h"
#import "surfaces/sfcombination.h"
#import "surfaces/sfproperties.h"

static NSArray* orientabilityText;
static NSArray* compactnessText;
static NSArray* boundaryText;
static NSMutableCharacterSet* eulerSeparators;

#pragma mark - Property-based filter

@interface FilterPropertiesViewController () <UITextFieldDelegate>
@property (weak, nonatomic) IBOutlet UISegmentedControl *orientability;
@property (weak, nonatomic) IBOutlet UILabel *orientabilityExpln;
@property (weak, nonatomic) IBOutlet UISegmentedControl *compactness;
@property (weak, nonatomic) IBOutlet UILabel *compactnessExpln;
@property (weak, nonatomic) IBOutlet UISegmentedControl *boundary;
@property (weak, nonatomic) IBOutlet UILabel *boundaryExpln;
@property (weak, nonatomic) IBOutlet UITextField *euler;
@property (weak, nonatomic) IBOutlet UILabel *eulerExpln;
@end

@implementation FilterPropertiesViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    if (! orientabilityText)
        [FilterPropertiesViewController initArrays];

    regina::NSurfaceFilterProperties* f = static_cast<regina::NSurfaceFilterProperties*>(self.packet);
    self.orientability.selectedSegmentIndex = [FilterPropertiesViewController selectionFromSet:f->getOrientability()];
    self.compactness.selectedSegmentIndex = [FilterPropertiesViewController selectionFromSet:f->getCompactness()];
    self.boundary.selectedSegmentIndex = [FilterPropertiesViewController selectionFromSet:f->getRealBoundary()];

    [self updateEulerDisplay];
    self.euler.delegate = self;

    // Update the description labels.
    // This will not trigger a change event on the packet, since the various
    // setFoo(...) routines only fire a change event if the new value differs from the old.
    [self orientabilityChanged:nil];
    [self compactnessChanged:nil];
    [self boundaryChanged:nil];
}

- (void)updateEulerDisplay
{
    const std::set<regina::NLargeInteger>& ECs = static_cast<regina::NSurfaceFilterProperties*>(self.packet)->getECs();
    if (ECs.empty()) {
        self.euler.text = @"";
        self.eulerExpln.text = @"No restrictions on Euler characteristic.";
    } else if (ECs.size() == 1) {
        self.euler.text = [NSString stringWithUTF8String:(*ECs.begin()).stringValue().c_str()];
        self.eulerExpln.text = @"The surface must have this exact Euler characteristic.";
    } else {
        std::set<regina::NLargeInteger>::reverse_iterator it = ECs.rbegin();
        NSMutableString* ans = [NSMutableString stringWithUTF8String:(*it).stringValue().c_str()];
        for (++it; it != ECs.rend(); ++it)
            [ans appendFormat:@", %s", (*it).stringValue().c_str()];
        self.euler.text = ans;
        self.eulerExpln.text = @"The surface must have one of these Euler characteristics.";
    }
}

- (IBAction)orientabilityChanged:(id)sender {
    int selection = self.orientability.selectedSegmentIndex;
    self.orientabilityExpln.text = orientabilityText[selection];
    static_cast<regina::NSurfaceFilterProperties*>(self.packet)->setOrientability([FilterPropertiesViewController setFromSelection:selection]);
}

- (IBAction)compactnessChanged:(id)sender {
    int selection = self.compactness.selectedSegmentIndex;
    self.compactnessExpln.text = compactnessText[selection];
    static_cast<regina::NSurfaceFilterProperties*>(self.packet)->setCompactness([FilterPropertiesViewController setFromSelection:selection]);
}

- (IBAction)boundaryChanged:(id)sender {
    int selection = self.boundary.selectedSegmentIndex;
    self.boundaryExpln.text = boundaryText[selection];
    static_cast<regina::NSurfaceFilterProperties*>(self.packet)->setRealBoundary([FilterPropertiesViewController setFromSelection:selection]);
}

+ (regina::NBoolSet)setFromSelection:(int)selection
{
    switch (selection) {
        case 0: return regina::NBoolSet::sBoth;
        case 1: return regina::NBoolSet::sTrue;
        case 2: return regina::NBoolSet::sFalse;
        default: return regina::NBoolSet::sNone;
    }
}

+ (int)selectionFromSet:(const regina::NBoolSet&)set
{
    if (set == regina::NBoolSet::sBoth)
        return 0;
    if (set == regina::NBoolSet::sTrue)
        return 1;
    if (set == regina::NBoolSet::sFalse)
        return 2;
    // Hmm?
    NSLog(@"Filter-by-properties: some property was set to sNone, changing to sBoth.");
    return 0;
}

+ (void)initArrays
{
    orientabilityText = @[@" ",
                          @"The surface must be orientable.",
                          @"The surface must be non-orientable."];
    compactnessText = @[@" ",
                        @"The surface must be compact (not spun-normal).",
                        @"The surface must be spun-normal (i.e., have infinitely many triangles)."];
    boundaryText = @[@" ",
                     @"The surface must have boundary edges.",
                     @"The surface cannot have boundary edges."];
}

- (void)endEditing {
    // As a consequence, this calls eulerEditEnded, which is where the real work happens.
    [self.euler resignFirstResponder];
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
    // Extract the set of Euler characteristics.
    if (eulerSeparators == nil) {
        eulerSeparators = [NSMutableCharacterSet whitespaceAndNewlineCharacterSet];
        [eulerSeparators addCharactersInString:@",()"];
    }

    std::set<regina::NLargeInteger> set;
    bool valid;
    for (NSString* eulerStr in [textField.text componentsSeparatedByCharactersInSet:eulerSeparators]) {
        // Multiple spaces will result in empty strings.
        if (eulerStr.length == 0)
            continue;

        regina::NLargeInteger euler([eulerStr UTF8String], 10, &valid);
        if (! valid) {
            UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Euler Characteristics Must Be Integers"
                                                            message:@"Please enter a list of allowed Euler characteristics, separated by commas or spaces."
                                                           delegate:nil
                                                  cancelButtonTitle:@"Close"
                                                  otherButtonTitles:nil];
            [alert show];

            // Reread the list from what is stored in the packet.
            [self updateEulerDisplay];
            return;
        }
        set.insert(euler);
    }

    regina::NSurfaceFilterProperties* f = static_cast<regina::NSurfaceFilterProperties*>(self.packet);
    f->setECs(set);
    [self updateEulerDisplay];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [self.euler resignFirstResponder];
    return NO;
}

@end

#pragma mark - Combination-based filter

@interface FilterCombinationViewController ()
@property (weak, nonatomic) IBOutlet UITableView *children;
@property (weak, nonatomic) IBOutlet UISegmentedControl *type;
@end

@implementation FilterCombinationViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    regina::NSurfaceFilterCombination* c = static_cast<regina::NSurfaceFilterCombination*>(self.packet);
}

@end
