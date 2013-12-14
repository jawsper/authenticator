//
//  utils.c
//  Pebble Authenticator
//
//  Created by Didier Arenzana on 14/12/13.

//

#include <pebble.h>

void log_handler_called(char *name) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "handler called: %s", name);
}

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
                                          const char *system_font_name) {
    TextLayer *layer=text_layer_create(frame);
	text_layer_set_text_color(      layer, foreground);
	text_layer_set_background_color(layer, background);
    text_layer_set_text_alignment(  layer, alignment);
	text_layer_set_font(layer, fonts_get_system_font(system_font_name));
    layer_add_child(parent, text_layer_get_layer(layer));
    return layer ;
}

