//
//  utils.h
//  Pebble Authenticator
//
//  Created by Didier Arenzana on 14/12/13.
//

#ifndef Pebble_Authenticator_utils_h
#define Pebble_Authenticator_utils_h

void log_handler_called(char *name) ;

/** creates a text layer and set its options
 * @param parent the parent Layer to attach the newly created layer to
 * @param frame  size of the layer frame
 * @param foreground foreground color
 * @param background background color
 * @param alignment  text alignment
 * @param system_font_name name of the system font to use
 */
TextLayer *text_layer_create_with_options(
                                          Layer *parent,
                                          GRect frame,
                                          GColor foreground,
                                          GColor background,
                                          GTextAlignment alignment,
                                          const char *system_font_name);
#endif
