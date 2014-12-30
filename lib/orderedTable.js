// DeviceJS
// (c) WigWag Inc 2014

/**
 * @module utils
 */

var _ = require('./underscore.js');
var UUID = require('node-uuid');

// a simple ordered hash table of callbacks (or anything for that matter)
/**
 * @class orderedTable
 * @constructor
 */
var orderedTable = function() {
    this.cbs = {}; 
    this.cborder = [];
    var count = 0;

    var that=this;
    /**
     * Add one or more items to the ordered list - to the back of the list. The items are added in order of their parameter list.
     * @method add
     * @param {any} item* Add items to the list
     * @return {string|array} If one item is added, a single unique string ID is returned. If more than one is added, an array of strings is returned. 
     * These ids can later be used to <code>remove</code> the items.
     */
    this.add = function() {
    	var ret = [];
    	for(var n=0;n<arguments.length;n++) {
            var id = UUID.v1();
            ret.push(id);
            that.cbs[id] = arguments[n];
            that.cborder.push(id);
            count++;
        }
        if(ret.length == 1)
            return ret[0];
        else if (ret.length == 0)
            return null;
        else
            return ret;
    }


    this.transferTo = function(table) {
        
    }


    /**
     * Replace or add an item with a specified ID.<br>
     * <i>Normally orderedTable generates an ID for each item. This function requires you to pass in a unique ID. If this is the ID to an existing item
     * the item is simply replaced.</i>
     * @method replaceAdd
     * @param {string} id The ID to use to uniquely reference the item.
     * @param {any} item The item to store in the table
     * @return {boolean} returns <code>item</code> if the item already existed, or <code>null</code> otherwise.
     */
    this.replaceAdd = function(id,item) {
        var exists = null;
        for(var n=0;n<that.cborder.length;n++) {
            if(that.cborder[n] == id)
                exists = that.cborder[n];
        }       
        if(!exists) that.cborder.push(id);
        that.cbs[id] = item;
        return exists;
    }


    /**
     * Looks up one or more items by their ID
     * @return {array|any}  
     */
    this.lookup = function() {
    	var ret = [];
    	for(var n=0;n<arguments.length;n++) {
    		if(that.cbs[arguments[n]])
    			ret.push(that.cbs[arguments[n]]);
        }
        if(ret.length == 1)
            return ret[0];
        else if (ret.length == 0)
            return null;
        else
            return ret;
    }

    this.lookupByOrder = function(o) {
        var ret = undefined;
        var i = that.cborder[o];
        if(i) {
            ret = that.cbs[i];
        }
        return ret;
    }

    /**
     * Add one or more items to the ordered list - adding each item in front of all other items.
     * @method addToFront
     * @param {any} item* Add items to the list. If more than one item, the last item in the arguments will be the first item in the list.
     * @return {string|array} If one item is added, a single unique string ID is returned. If more than one is added, an array of strings is returned. 
     * These ids can later be used to <code>remove</code> the items.
     */
    this.addToFront = function() {
    	var ret = [];
        for(var n=0;n<arguments.length;n++) {
            var id = UUID.v1();
            ret.splice(0,0,id);
            that.cbs[id] = arguments[n];
            that.cborder.splice(0,0,id);
            count++;
        }
        if(ret.length == 1)
            return ret[0];
        else if (ret.length == 0)
            return null;
        else
            return ret;
    }

    /**
     * Calls a function for each item in the list.
     * @method forEach
     * @param  {[type]} func The callback function.<br>
     * <code>callback(val,id,order)</code><br>
     * <code>id</code> is the the unique ID of the element in the list<br>
     * <code>order</code> is the zero-based order of the item in the list<br><br>
     * <code>this</code> for the <code>callback</code> is the item in the list<br><br>
     * NOTE: It is safe to use <code>orderedTable.remove()</code> during this call.
     */
    this.forEach = function(func) {
    	var c = 0;
    	for(var n=0;n<that.cborder.length;n++) {
    		if(that.cbs[that.cborder[n]]) {// if it is still in the table, return it
    			func.call(undefined,that.cbs[that.cborder[n]],that.cborder[n],c);
    			c++;
    		}
    	}
    } 

    /**
     * Remove items from the ordered list.
     * @method remove
     * @param {string} id* one or more id values. These value correspond to a certain item in the list. These item(s) will be removed.
     * @return {any|Array} A single item, null if nothing, or an Array of multiple items.
     */
    this.remove = function() {
    	var ret = [];
    	for(var n=0;n<arguments.length;n++) {
    		if (that.cbs[arguments[n]]) {
    			ret.push(that.cbs[arguments[n]]);
    			delete that.cbs[arguments[n]];  // the remaining cborder entry is just ignored
                count--;
                var index = that.cborder.indexOf(arguments[n]);
                if(index >= 0) {
                    that.cborder.splice(index,1);
                }
    		}
    	}
        if(ret.length == 1)
            return ret[0];
        else if (ret.length == 0)
            return null;
        else
            return ret;
    }


    /**
     * Removes all elements, clears list.
     * @method removeAll
     */
    this.removeAll = function() {
        that.cbs = {}; 
        that.cborder = [];
        count = 0;
    }


    /**
     * Returns an ordered list of the items in the table, in the same order the items were put in (FIFO).
     * @method getList
     * @return an array of the items in their original order.
     */
    this.getList = function() {
    	var ret = [];
    	for(var n=0;n<that.cborder.length;n++) {
    		if(that.cbs[that.cborder[n]]) // if it is still in the table, return it
    			ret.push(that.cbs[that.cborder[n]]);
    	}
    	return ret;
    }
    /**
     * Returns an ordered list of the items in the table, in the reverse of the order the items were put in (FILO).
     * @method getReverseList
     * @return an array of the items in their reverse order.
     */
    this.getReverseList = function() {
    	var ret = [];
    	for(var n=that.cborder.length;n>=0;n--) {
    		if(that.cbs[that.cborder[n]]) // if it is still in the table, return it
    			ret.push(that.cbs[that.cborder[n]]);
    	}
    	return ret;
    }

    /**
     * Removes an item by its order in the table.
     * @param  {[type]} n The zero-based order of the item to remove
     * @return {any} Returns undefined if order value too high and nothing removed.
     */
    this.removeByOrder = function(n) {
        var ret = undefined;
        if(that.cborder[n]) {
            ret = that.cbs[that.cborder[n]];
            delete that.cbs[that.cborder[n]];
            that.cborder.splice(n,1); // remove from order index
            count--;
        }
        return ret;
    }


    /**
     * Replace an item by order. An item must already exist for this to work.<br>
     * @method replaceAdd
     * @param {string} n The zero-based order of the item
     * @param {any} item The item to store in the table
     * @return {any} the original item that was replaced
     */
    this.replaceByOrder = function(n,item) {
        var ret = undefined;
        var exists = null;
        var id = that.cborder[n];
        if(id && that.cbs[id]) {
            ret = that.cbs[id];
            that.cbs[id] = item;
        }
        return ret;
    }


    this.length = function() {
        return count;
    }
}

exports = module.exports = orderedTable;
