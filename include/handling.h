/*
    handling.h
    Copyright (C) 2021 Richard Knight


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
 */
#include <plugin.h>

const char* handleIdentifier = "::%s%d";

/*
 * Obtain global object of typename from global variables by handle
 */
template <typename T>
T* getHandle(csnd::Csound* csound, MYFLT handle, const char* name) {
    char buffer[32];
    snprintf(buffer, 32, handleIdentifier, name, (int)handle);
    return (T*) csound->query_global_variable(buffer);  
}


/*
 * Create global object of typename in global variables, returning handle
 */
template <typename T>
MYFLT createHandle(csnd::Csound* csound, T** data, const char* name) {
    char buffer[32];
    int handle = 0;
    snprintf(buffer, 32, handleIdentifier, name, handle);
    while ((*data = (T*) csound->query_global_variable(buffer)) != NULL) {
        snprintf(buffer, 32, handleIdentifier, name, ++handle);
    }
    csound->create_global_variable(buffer, sizeof(T));
    *data = (T*) csound->query_global_variable(buffer);
    
    return FL(handle);
}

/*
 * Destroy global object
 */
void destroyHandle(csnd::Csound* csound, MYFLT handle, const char* name) {
    char buffer[32];
    snprintf(buffer, 32, handleIdentifier, name, (int)handle);
    csound->destroy_global_variable(buffer);
}


